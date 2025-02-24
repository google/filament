# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import base64
import json
import logging
import re
import socket
import time
import traceback

from telemetry.core import exceptions
from telemetry import decorators
from telemetry.internal.backends.chrome_inspector import inspector_websocket
from telemetry.internal.backends.chrome_inspector import websocket
from tracing.trace_data import trace_data as trace_data_module


class TracingUnsupportedException(exceptions.Error):
  pass


class TracingTimeoutException(exceptions.Error):
  pass


class TracingUnrecoverableException(exceptions.Error):
  pass


class TracingHasNotRunException(exceptions.Error):
  pass


class TracingUnexpectedResponseException(exceptions.Error):
  pass


class ClockSyncResponseException(exceptions.Error):
  pass


class TraceBufferDataLossException(exceptions.Error):
  pass


class _DevToolsStreamReader():
  def __init__(self, inspector_socket, stream_handle, trace_handle):
    """Constructor for the stream reader that reads trace data over a stream.

    Args:
      inspector_socket: An inspector_websocket.InspectorWebsocket instance.
      stream_handle: A handle, as returned by Chrome, from where to read the
        trace data.
      trace_handle: A Python file-like object where to write the trace data.
    """
    self._inspector_websocket = inspector_socket
    self._stream_handle = stream_handle
    self._trace_handle = trace_handle
    self._callback = None

  def Read(self, callback):
    # Do not allow the instance of this class to be reused, as
    # we only read data sequentially at the moment, so a stream
    # can only be read once.
    assert not self._callback
    self._callback = callback
    self._ReadChunkFromStream()
    # The below is not a typo -- queue one extra read ahead to avoid latency.
    self._ReadChunkFromStream()

  def _ReadChunkFromStream(self):
    # Limit max block size to avoid fragmenting memory in sock.recv(),
    # (see https://github.com/liris/websocket-client/issues/163 for details)
    req = {'method': 'IO.read', 'params': {
        'handle': self._stream_handle, 'size': 32768}}
    self._inspector_websocket.AsyncRequest(req, self._GotChunkFromStream)

  def _GotChunkFromStream(self, response):
    # Quietly discard responses from reads queued ahead after EOF.
    if self._trace_handle is None:
      return
    if 'error' in response:
      raise TracingUnrecoverableException(
          'Reading trace failed: %s' % response['error']['message'])
    result = response['result']
    # Convert the obtained unicode trace data to raw bytes..
    data_chunk = result['data'].encode('utf8')
    if result.get('base64Encoded', False):
      data_chunk = base64.b64decode(data_chunk)
    self._trace_handle.write(data_chunk)

    if not result.get('eof', False):
      self._ReadChunkFromStream()
      return
    req = {'method': 'IO.close', 'params': {'handle': self._stream_handle}}
    self._inspector_websocket.SendAndIgnoreResponse(req)
    self._trace_handle.close()
    self._trace_handle = None
    self._callback()


class TracingBackend():

  _TRACING_DOMAIN = 'Tracing'

  def __init__(self, inspector_socket, startup_tracing_config=None):
    self._inspector_websocket = inspector_socket
    self._inspector_websocket.RegisterDomain(
        self._TRACING_DOMAIN, self._NotificationHandler)
    self._is_tracing_running = False
    self._can_collect_data = False
    self._has_received_all_tracing_data = False
    self._trace_data_builder = None
    self._data_loss_occurred = False
    if startup_tracing_config is not None:
      self._TakeOwnershipOfTracingSession(startup_tracing_config)

  @property
  def is_tracing_running(self):
    return self._is_tracing_running

  def _TakeOwnershipOfTracingSession(self, config):
    # Startup tracing should already be running, but we still need to send a
    # Tracing.start command for DevTools to become owner of the tracing session
    # and to update the transfer settings.
    # This also ensures that tracing data from early startup is flushed to the
    # tracing service before the thread-local buffers for startup tracing are
    # exhausted (crbug.com/914092).
    response = self._SendTracingStartRequest(
        trace_format=config.chrome_trace_config.trace_format)
    # Note: we do in fact expect an "error" response as the call, in addition
    # to updating the transfer settings for trace collection, also serves to
    # confirm the fact that startup tracing is in place. In fact, it would be
    # an error if this request succeeds.
    error_message = response.get('error', {}).get('message', '')
    if not re.match(r'Tracing.*already.*started', error_message):
      raise TracingUnexpectedResponseException(
          'Tracing.start failed to confirm startup tracing:\n' +
          json.dumps(response, indent=2))
    logging.info('Successfully confirmed startup tracing is in place.')
    self._is_tracing_running = True

  def StartTracing(self, chrome_trace_config, transfer_mode=None, timeout=20):
    """Starts tracing if not already started.

    Args:
      chrome_trace_config: A chrome_trace_config.ChromeTraceConfig instance.
      transfer_mode: The transfer mode for chrome tracing. If set to default,
        'ReturnAsStream' mode will be used.
      timeout: Time waited for websocket to receive a response.

    Returns:
      True if tracing was not running and False otherwise.
    """
    if self.is_tracing_running:
      return False
    assert not self._can_collect_data, 'Data not collected from last trace.'
    # Reset collected tracing data from previous tracing calls.
    self._has_received_all_tracing_data = False
    self._data_loss_occurred = False
    if not self.IsTracingSupported():
      raise TracingUnsupportedException(
          'Chrome tracing not supported for this app.')

    response = self._SendTracingStartRequest(
        transfer_mode=transfer_mode,
        trace_config=chrome_trace_config.GetChromeTraceConfigForDevTools(),
        trace_format=chrome_trace_config.trace_format,
        timeout=timeout)
    if 'error' in response:
      raise TracingUnexpectedResponseException(
          'Inspector returned unexpected response for Tracing.start:\n' +
          json.dumps(response, indent=2))
    logging.info('Successfully started tracing.')
    self._is_tracing_running = True
    return True

  def _SendTracingStartRequest(self, transfer_mode=None, trace_config=None,
                               trace_format=None, timeout=20):
    """Send a Tracing.start request and wait for a response.

    Args:
      trace_config: A dictionary speficying to Chrome what should be traced.
        For example: {'recordMode': 'recordUntilFull', 'includedCategories':
        ['x', 'y'], ...}. It is required to start tracing via DevTools, and
        should be omitted if startup tracing was already started.
      trace_format: An optional string identifying the requested format in which
        to stream the recorded trace back to the client. Chrome currently
        defaults to JSON if omitted.

    Returns:
      A dictionary suitable to pass as a DevTools request.
    """
    # Using 'gzip' compression reduces the amount of data transferred over
    # websocket. This reduces the time waiting for all data to be received,
    # especially when the test is running on an android device. Using
    # compression can save upto 10 seconds (or more) for each story.
    params = {
        'transferMode': transfer_mode or 'ReturnAsStream',
        'traceConfig': trace_config or {}}
    if params['transferMode'] == 'ReturnAsStream':
      params['streamCompression'] = 'gzip'
      if trace_format is not None:
        params['streamFormat'] = trace_format
    request = {'method': 'Tracing.start', 'params': params}
    return self._inspector_websocket.SyncRequest(request, timeout)

  def RecordClockSyncMarker(self, sync_id):
    assert self.is_tracing_running, 'Tracing must be running to clock sync.'
    req = {
        'method': 'Tracing.recordClockSyncMarker',
        'params': {
            'syncId': sync_id
        }
    }
    rc = self._inspector_websocket.SyncRequest(req, timeout=2)
    if 'error' in rc:
      raise ClockSyncResponseException(rc['error']['message'])

  def StopTracing(self):
    """Stops tracing and pushes results to the supplied TraceDataBuilder.

    If this is called after tracing has been stopped, trace data from the last
    tracing run is pushed.
    """
    if not self.is_tracing_running:
      raise TracingHasNotRunException()
    req = {'method': 'Tracing.end'}
    response = self._inspector_websocket.SyncRequest(req, timeout=2)
    if 'error' in response:
      raise TracingUnexpectedResponseException(
          'Inspector returned unexpected response for '
          'Tracing.end:\n' + json.dumps(response, indent=2))

    logging.info('Successfully stopped tracing.')
    self._is_tracing_running = False
    self._can_collect_data = True

  def DumpMemory(self, timeout=None, detail_level=None, deterministic=False):
    """Dumps memory.

    Args:
      timeout: If not specified defaults to 20 minutes.
      detail_level: Level of detail in memory dump. One of ['background',
        'light', 'detailed']. Default is 'detailed'.

    Returns:
      GUID of the generated dump if successful, None otherwise.

    Raises:
      TracingTimeoutException: If more than |timeout| seconds has passed
      since the last time any data is received.
      TracingUnrecoverableException: If there is a websocket error.
      TracingUnexpectedResponseException: If the response contains an error
      or does not contain the expected result.
    """
    params = {}
    if detail_level:
      assert detail_level in ['background', 'light', 'detailed']
      params = {'levelOfDetail': detail_level}
    if deterministic:
      params['deterministic'] = True
    request = {'method': 'Tracing.requestMemoryDump', 'params': params}
    if timeout is None:
      timeout = 1200  # 20 minutes.
    try:
      response = self._inspector_websocket.SyncRequest(request, timeout)
    except inspector_websocket.WebSocketException as err:
      if issubclass(
          err.websocket_error_type, websocket.WebSocketTimeoutException):
        raise TracingTimeoutException(
            'Exception raised while sending a Tracing.requestMemoryDump '
            'request:\n' + traceback.format_exc()) from err
      raise TracingUnrecoverableException(
          'Exception raised while sending a Tracing.requestMemoryDump '
          'request:\n' + traceback.format_exc()) from err
    except (socket.error,
            inspector_websocket.WebSocketDisconnected) as err:
      raise TracingUnrecoverableException(
          'Exception raised while sending a Tracing.requestMemoryDump '
          'request:\n' + traceback.format_exc()) from err
    dump_id = None
    try:
      if response['result']['success'] and 'error' not in response:
        dump_id = response['result']['dumpGuid']
    except KeyError:
      pass  # If any of the keys are missing, there is an error and no dump_id.
    if not dump_id:
      raise TracingUnexpectedResponseException(
          'Inspector returned unexpected response for '
          'Tracing.requestMemoryDump:\n' + json.dumps(response, indent=2))
    return dump_id

  def CollectTraceData(self, trace_data_builder, timeout=60):
    if not self._can_collect_data:
      raise Exception('Cannot collect before tracing is finished.')
    self._CollectTracingData(trace_data_builder, timeout)
    self._can_collect_data = False

  def _CollectTracingData(self, trace_data_builder, timeout):
    """Collects tracing data. Assumes that Tracing.end has already been sent.

    Args:
      trace_data_builder: An instance of TraceDataBuilder to put results into.
      timeout: The timeout in seconds.

    Raises:
      TracingTimeoutException: If more than |timeout| seconds has passed
      since the last time any data is received.
      TracingUnrecoverableException: If there is a websocket error.
    """
    before_time = time.time()
    start_time = time.time()
    self._trace_data_builder = trace_data_builder
    try:
      while True:
        try:
          self._inspector_websocket.DispatchNotifications(timeout)
          start_time = time.time()
        except inspector_websocket.WebSocketException as err:
          if not issubclass(
              err.websocket_error_type, websocket.WebSocketTimeoutException):
            raise TracingUnrecoverableException(
                'Exception raised while collecting tracing data:\n' +
                traceback.format_exc()) from err
        except socket.error as err:
          raise TracingUnrecoverableException(
              'Exception raised while collecting tracing data:\n' +
              traceback.format_exc()) from err

        if self._has_received_all_tracing_data:
          # Only raise this exception after collecting all the data to aid
          # debugging.
          if self._data_loss_occurred:
            raise TraceBufferDataLossException(
                'The trace buffer has been overrun and data loss has occurred. '
                'Chrome\'s tracing is stored in a ring buffer. When it runs '
                'out of space, it will start deleting trace information from '
                'the start. Data loss can cause some unexpected problems in '
                'the metrics calculation implementation. For example, metrics '
                'depend on the clock sync marker existing. For that reason, it '
                'is better to hard fail here than to let metrics calculations '
                'fail in a more cryptic way.\n'
                'There are several ways to prevent this error:\n'
                '1. Shorten your story so that it does not run long enough to '
                'overflow the trace buffer.\n'
                '2. Enable fewer trace categories to generate less data.\n'
                '3. Increase the trace buffer size.')
          break

        elapsed_time = time.time() - start_time
        if elapsed_time > timeout:
          trace_parts = [
            name for name, _ in self._trace_data_builder.IterTraceParts()]
          raise TracingTimeoutException(
              'Only received partial trace data due to timeout after %s '
              'seconds. If the trace data is big, you may want to increase '
              'the timeout amount.\nTrace Parts: %s' % (
                  elapsed_time, trace_parts))
    finally:
      self._trace_data_builder = None
    logging.info('Successfully collected all trace data after %f seconds.'
                 % (time.time() - before_time))

  def _NotificationHandler(self, res):
    if res.get('method') == 'Tracing.dataCollected':
      value = res.get('params', {}).get('value')
      self._trace_data_builder.AddTraceFor(
          trace_data_module.CHROME_TRACE_PART,
          {'traceEvents': value})
    elif res.get('method') == 'Tracing.tracingComplete':
      params = res.get('params', {})
      # TODO(crbug.com/948412): Start requiring a value for dataLossOccurred
      # once we stop supporting Chrome M76 (which was the last version that
      # did not return this as a required parameter).
      self._data_loss_occurred = params.get('dataLossOccurred', False)
      stream_handle = params.get('stream')
      if not stream_handle:
        self._has_received_all_tracing_data = True
        return
      trace_handle = self._trace_data_builder.OpenTraceHandleFor(
          trace_data_module.CHROME_TRACE_PART,
          suffix=_GetTraceFileSuffix(params))
      reader = _DevToolsStreamReader(
          self._inspector_websocket, stream_handle, trace_handle)
      reader.Read(self._ReceivedAllTraceDataFromStream)

  def _ReceivedAllTraceDataFromStream(self):
    self._has_received_all_tracing_data = True

  def Close(self):
    self._inspector_websocket.UnregisterDomain(self._TRACING_DOMAIN)
    self._inspector_websocket = None

  @decorators.Cache
  def IsTracingSupported(self):
    req = {'method': 'Tracing.hasCompleted'}
    res = self._inspector_websocket.SyncRequest(req, timeout=10)
    return not res.get('response')


def _GetTraceFileSuffix(params):
  suffix = '.' + params.get('traceFormat', 'json')
  if suffix == '.proto':
    suffix = '.pb'
  if params.get('streamCompression') == 'gzip':
    suffix += '.gz'
  return suffix
