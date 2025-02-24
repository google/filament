# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import re
import threading

import six

if six.PY3:
  import queue # pylint: disable=wrong-import-order
else:
  import Queue as queue # pylint: disable=wrong-import-order

# Log marker containing SurfaceTexture timestamps.
_SURFACE_TEXTURE_TIMESTAMPS_MESSAGE = 'SurfaceTexture update timestamps'
_SURFACE_TEXTURE_TIMESTAMP_RE = r'\d+'


class SurfaceStatsCollector(object):
  """Collects surface stats for a SurfaceView from the output of SurfaceFlinger.

  Args:
    device: A DeviceUtils instance.
  """

  def __init__(self, device):
    self._device = device
    self._collector_thread = None
    self._surface_before = None
    self._get_data_event = None
    self._data_queue = None
    self._stop_event = None
    self._warn_about_empty_data = True

  def DisableWarningAboutEmptyData(self):
    self._warn_about_empty_data = False

  def Start(self):
    assert not self._collector_thread

    if self._ClearSurfaceFlingerLatencyData():
      self._get_data_event = threading.Event()
      self._stop_event = threading.Event()
      self._data_queue = queue.Queue()
      self._collector_thread = threading.Thread(target=self._CollectorThread)
      self._collector_thread.start()
    else:
      raise Exception('SurfaceFlinger not supported on this device.')

  def Stop(self):
    assert self._collector_thread
    (refresh_period, timestamps) = self._GetDataFromThread()
    if self._collector_thread:
      self._stop_event.set()
      self._collector_thread.join()
      self._collector_thread = None
    return (refresh_period, timestamps)

  def _CollectorThread(self):
    last_timestamp = 0
    timestamps = []
    retries = 0

    while not self._stop_event.is_set():
      self._get_data_event.wait(1)
      try:
        refresh_period, new_timestamps = self._GetSurfaceFlingerFrameData()
        if refresh_period is None or timestamps is None:
          retries += 1
          if retries < 3:
            continue
          if last_timestamp:
            # Some data has already been collected, but either the app
            # was closed or there's no new data. Signal the main thread and
            # wait.
            self._data_queue.put((None, None))
            self._stop_event.wait()
            break
          raise Exception('Unable to get surface flinger latency data')

        timestamps += [
            timestamp for timestamp in new_timestamps
            if timestamp > last_timestamp
        ]
        if len(timestamps):
          last_timestamp = timestamps[-1]

        if self._get_data_event.is_set():
          self._get_data_event.clear()
          self._data_queue.put((refresh_period, timestamps))
          timestamps = []
      except Exception as e:
        # On any error, before aborting, put the exception into _data_queue to
        # prevent the main thread from waiting at _data_queue.get() infinitely.
        self._data_queue.put(e)
        raise

  def _GetDataFromThread(self):
    self._get_data_event.set()
    ret = self._data_queue.get()
    if isinstance(ret, Exception):
      raise ret
    return ret

  def _ClearSurfaceFlingerLatencyData(self):
    """Clears the SurfaceFlinger latency data.

    Returns:
      True if SurfaceFlinger latency is supported by the device, otherwise
      False.
    """
    # The command returns nothing if it is supported, otherwise returns many
    # lines of result just like 'dumpsys SurfaceFlinger'.
    results = self._device.RunShellCommand(
        ['dumpsys', 'SurfaceFlinger', '--latency-clear', 'SurfaceView'],
        check_return=True)
    return not results

  def GetSurfaceFlingerPid(self):
    try:
      # Returns the first matching PID found.
      return next(p.pid for p in self._device.ListProcesses('surfaceflinger'))
    except StopIteration:
      raise Exception('Unable to get surface flinger process id')

  def _GetSurfaceViewWindowName(self):
    results = self._device.RunShellCommand(
        ['dumpsys', 'SurfaceFlinger', '--list'], check_return=True)
    for window_name in results:
      if window_name.startswith('SurfaceView'):
        return window_name
    return None

  def _GetSurfaceFlingerFrameData(self):
    """Returns collected SurfaceFlinger frame timing data.

    Returns:
      A tuple containing:
      - The display's nominal refresh period in milliseconds.
      - A list of timestamps signifying frame presentation times in
        milliseconds.
      The return value may be (None, None) if there was no data collected (for
      example, if the app was closed before the collector thread has finished).
    """
    window_name = self._GetSurfaceViewWindowName()
    command = ['dumpsys', 'SurfaceFlinger', '--latency']
    # Even if we don't find the window name, run the command to get the refresh
    # period.
    if window_name:
      command.append(window_name)
    output = self._device.RunShellCommand(command, check_return=True)
    return ParseFrameData(output, parse_timestamps=bool(window_name))


def to_long_int(val):
  """Cast val to a long int type."""
  # pylint: disable=undefined-variable
  return long(val) if six.PY2 else int(val)
  # pylint: enable=undefined-variable


def ParseFrameData(lines, parse_timestamps):
  # adb shell dumpsys SurfaceFlinger --latency <window name>
  # prints some information about the last 128 frames displayed in
  # that window.
  # The data returned looks like this:
  # 16954612
  # 7657467895508   7657482691352   7657493499756
  # 7657484466553   7657499645964   7657511077881
  # 7657500793457   7657516600576   7657527404785
  # (...)
  #
  # The first line is the refresh period (here 16.95 ms), it is followed
  # by 128 lines w/ 3 timestamps in nanosecond each:
  # A) when the app started to draw
  # B) the vsync immediately preceding SF submitting the frame to the h/w
  # C) timestamp immediately after SF submitted that frame to the h/w
  #
  # The difference between the 1st and 3rd timestamp is the frame-latency.
  # An interesting data is when the frame latency crosses a refresh period
  # boundary, this can be calculated this way:
  #
  # ceil((C - A) / refresh-period)
  #
  # (each time the number above changes, we have a "jank").
  # If this happens a lot during an animation, the animation appears
  # janky, even if it runs at 60 fps in average.
  results = []
  for line in lines:
    # Skip over lines with anything other than digits and whitespace.
    if re.search(r'[^\d\s]', line):
      logging.warning('unexpected output: %s', line)
    else:
      results.append(line)
  if not results:
    return None, None

  timestamps = []
  nanoseconds_per_millisecond = 1e6
  refresh_period = to_long_int(results[0]) / nanoseconds_per_millisecond

  if not parse_timestamps:
    return refresh_period, timestamps

  # If a fence associated with a frame is still pending when we query the
  # latency data, SurfaceFlinger gives the frame a timestamp of INT64_MAX.
  # Since we only care about completed frames, we will ignore any timestamps
  # with this value.
  pending_fence_timestamp = (1 << 63) - 1

  for line in results[1:]:
    if not line:
      continue

    fields = line.split()
    if len(fields) != 3:
      logging.warning('Unexpected line: %s', line)
      continue

    timestamp = to_long_int(fields[1])
    if timestamp == pending_fence_timestamp:
      continue

    timestamp /= nanoseconds_per_millisecond
    timestamps.append(timestamp)

  return refresh_period, timestamps
