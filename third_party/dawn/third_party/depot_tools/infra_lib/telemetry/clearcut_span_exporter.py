# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Defines the telemetry exporter for exporting to ClearCut."""

import datetime
import logging
import time
import urllib.error
import urllib.request

from typing import Callable, Dict, Optional, Pattern, Sequence, Tuple
from google.protobuf import (
    json_format,
    message as proto_msg,
    struct_pb2,
)
from opentelemetry import trace as otel_trace_api
from opentelemetry.sdk import (
    trace as otel_trace_sdk,
    resources as otel_resources,
)
from opentelemetry.sdk.trace import export as otel_export
from opentelemetry.util import types as otel_types

from . import anonymization
from . import detector
from .proto import trace_span_pb2
from .proto import clientanalytics_pb2

_DEFAULT_ENDPOINT = 'https://play.googleapis.com/log'
_DEFAULT_TIMEOUT = 15
_DEFAULT_FLUSH_TIMEOUT_MILLIS = 30000
_DEAULT_MAX_WAIT_SECS = 60
# Preallocated in Clearcut proto to cros Build.
_LOG_SOURCE = 2044
# Preallocated in Clearcut proto to Python clients.
_CLIENT_TYPE = 33
_DEFAULT_MAX_QUEUE_SIZE = 1000


class ClearcutSpanExporter(otel_export.SpanExporter):
    """Exports the spans to google http endpoint."""

    def __init__(
        self,
        endpoint: str = _DEFAULT_ENDPOINT,
        timeout: int = _DEFAULT_TIMEOUT,
        max_wait_secs: int = _DEAULT_MAX_WAIT_SECS,
        max_queue_size: int = _DEFAULT_MAX_QUEUE_SIZE,
        prefilter: Optional[Callable[[trace_span_pb2.TraceSpan],
                                     trace_span_pb2.TraceSpan]] = None,
    ) -> None:
        self._endpoint = endpoint
        self._timeout = timeout
        self._prefilter = prefilter or anonymization.AnonymizingFilter(
            anonymization.Anonymizer())
        self._log_source = _LOG_SOURCE
        self._next_request_dt = datetime.datetime.now()
        self._max_wait_secs = max_wait_secs
        self._queue = []
        self._max_queue_size = max_queue_size

    def export(
        self, spans: Sequence[otel_trace_sdk.ReadableSpan]
    ) -> otel_export.SpanExportResult:
        spans = (self._prefilter(self._translate_span(s)) for s in spans)
        self._queue.extend(spans)

        if len(self._queue) >= self._max_queue_size:
            return (otel_export.SpanExportResult.SUCCESS
                    if self._export_batch() else
                    otel_export.SpanExportResult.FAILURE)

        return otel_export.SpanExportResult.SUCCESS

    def shutdown(self) -> None:
        self.force_flush()

    def force_flush(self,
                    timeout_millis: int = _DEFAULT_FLUSH_TIMEOUT_MILLIS
                    ) -> bool:
        if self._queue:
            return self._export_batch(timeout=timeout_millis / 1000)

        return True

    def _translate_context(
            self, data: otel_trace_api.SpanContext
    ) -> trace_span_pb2.TraceSpan.Context:
        ctx = trace_span_pb2.TraceSpan.Context()
        ctx.trace_id = f'0x{otel_trace_api.format_trace_id(data.trace_id)}'
        ctx.span_id = f'0x{otel_trace_api.format_span_id(data.span_id)}'
        ctx.trace_state = repr(data.trace_state)
        return ctx

    def _translate_attributes(self,
                              data: otel_types.Attributes) -> struct_pb2.Struct:
        patch = {}
        for key, value in data.items():
            if isinstance(value, tuple):
                value = list(value)
            patch[key] = value

        struct = struct_pb2.Struct()
        try:
            struct.update(patch)
        except Exception as exception:
            logging.debug('Set attribute failed: %s', exception)
        return struct

    def _translate_span_attributes(
            self, data: otel_trace_sdk.ReadableSpan) -> struct_pb2.Struct:
        return self._translate_attributes(data.attributes)

    def _translate_links(
            self,
            data: otel_trace_sdk.ReadableSpan) -> trace_span_pb2.TraceSpan.Link:
        links = []

        for link_data in data.links:
            link = trace_span_pb2.TraceSpan.Link()
            link.context.MergeFrom(self._translate_context(link_data.context))
            link.attributes.MergeFrom(
                self._translate_attributes(link_data.attributes))
            links.append(link)

        return links

    def _translate_events(
            self, data: otel_trace_sdk.ReadableSpan
    ) -> trace_span_pb2.TraceSpan.Event:
        events = []
        for event_data in data.events:
            event = trace_span_pb2.TraceSpan.Event()
            event.event_time_millis = int(event_data.timestamp / 1e6)
            event.name = event_data.name
            event.attributes.MergeFrom(
                self._translate_attributes(event_data.attributes))
            events.append(event)
        return events

    def _translate_instrumentation_scope(
        self, data: otel_trace_sdk.ReadableSpan
    ) -> trace_span_pb2.TraceSpan.InstrumentationScope:
        instrumentation_scope = data.instrumentation_scope
        scope = trace_span_pb2.TraceSpan.InstrumentationScope()
        scope.name = instrumentation_scope.name
        scope.version = instrumentation_scope.version
        return scope

    def _translate_env(self, data: Dict[str, str]) -> Dict[str, str]:
        environ = {}
        for key, value in data.items():
            if key.startswith('process.env.'):
                key = key.split('process.env.')[1]
                environ[key] = value
        return environ

    def _translate_resource(
            self, data: otel_trace_sdk.ReadableSpan
    ) -> trace_span_pb2.TraceSpan.Resource:
        attrs = dict(data.resource.attributes)
        resource = trace_span_pb2.TraceSpan.Resource()
        resource.system.cpu = attrs.pop(detector.CPU_NAME, '')
        resource.system.host_architecture = attrs.pop(detector.CPU_ARCHITECTURE,
                                                      '')
        resource.system.os_name = attrs.pop(detector.OS_NAME, '')
        resource.system.os_version = attrs.pop(otel_resources.OS_DESCRIPTION,
                                               '')
        resource.system.os_type = attrs.pop(otel_resources.OS_TYPE, '')
        resource.process.pid = str(attrs.pop(otel_resources.PROCESS_PID, ''))
        resource.process.executable_name = attrs.pop(
            otel_resources.PROCESS_EXECUTABLE_NAME, '')
        resource.process.executable_path = attrs.pop(
            otel_resources.PROCESS_EXECUTABLE_PATH, '')
        resource.process.command = attrs.pop(otel_resources.PROCESS_COMMAND, '')
        resource.process.command_args.extend(
            attrs.pop(otel_resources.PROCESS_COMMAND_ARGS, []))
        resource.process.owner_is_root = (attrs.pop(
            otel_resources.PROCESS_OWNER, 9999) == 0)
        resource.process.runtime_name = attrs.pop(
            otel_resources.PROCESS_RUNTIME_NAME, '')
        resource.process.runtime_version = attrs.pop(
            otel_resources.PROCESS_RUNTIME_VERSION, '')
        resource.process.runtime_description = attrs.pop(
            otel_resources.PROCESS_RUNTIME_DESCRIPTION, '')
        resource.process.api_version = str(
            attrs.pop(detector.PROCESS_RUNTIME_API_VERSION, ''))
        resource.process.env.update(self._translate_env(attrs))
        resource.attributes.MergeFrom(self._translate_attributes(attrs))
        return resource

    def _translate_status(
            self, data: otel_trace_sdk.ReadableSpan
    ) -> trace_span_pb2.TraceSpan.Status:
        status = trace_span_pb2.TraceSpan.Status()

        if data.status.status_code == otel_trace_sdk.StatusCode.ERROR:
            status.status_code = (
                trace_span_pb2.TraceSpan.Status.StatusCode.STATUS_CODE_ERROR)
        else:
            status.status_code = (
                trace_span_pb2.TraceSpan.Status.StatusCode.STATUS_CODE_OK)

        if data.status.description:
            status.message = data.status.description

        return status

    def _translate_sdk(
        self, data: otel_trace_sdk.ReadableSpan
    ) -> trace_span_pb2.TraceSpan.TelemetrySdk:
        attrs = data.resource.attributes
        sdk = trace_span_pb2.TraceSpan.TelemetrySdk()
        sdk.name = attrs.get(otel_resources.TELEMETRY_SDK_NAME)
        sdk.version = attrs.get(otel_resources.TELEMETRY_SDK_VERSION)
        sdk.language = attrs.get(otel_resources.TELEMETRY_SDK_LANGUAGE)
        return sdk

    def _translate_kind(
            self,
            data: otel_trace_api.SpanKind) -> trace_span_pb2.TraceSpan.SpanKind:
        if data == otel_trace_api.SpanKind.INTERNAL:
            return trace_span_pb2.TraceSpan.SpanKind.SPAN_KIND_INTERNAL
        elif data == otel_trace_api.SpanKind.CLIENT:
            return trace_span_pb2.TraceSpan.SpanKind.SPAN_KIND_CLIENT
        elif data == otel_trace_api.SpanKind.SERVER:
            return trace_span_pb2.TraceSpan.SpanKind.SPAN_KIND_SERVER
        return trace_span_pb2.TraceSpan.SpanKind.SPAN_KIND_UNSPECIFIED

    def _translate_span(
            self,
            data: otel_trace_sdk.ReadableSpan) -> trace_span_pb2.TraceSpan:
        span = trace_span_pb2.TraceSpan()
        span.name = data.name
        span.context.MergeFrom(self._translate_context(data.get_span_context()))

        if isinstance(data.parent, otel_trace_api.Span):
            ctx = data.parent.context
            span.parent_span_id = (
                f'0x{otel_trace_api.format_span_id(ctx.span_id)}')
        elif isinstance(data.parent, otel_trace_api.SpanContext):
            span.parent_span_id = (
                f'0x{otel_trace_api.format_span_id(data.parent.span_id)}')

        span.start_time_millis = int(data.start_time / 1e6)
        span.end_time_millis = int(data.end_time / 1e6)
        span.span_kind = self._translate_kind(data.kind)
        span.instrumentation_scope.MergeFrom(
            self._translate_instrumentation_scope(data))
        span.events.extend(self._translate_events(data))
        span.links.extend(self._translate_links(data))
        span.attributes.MergeFrom(self._translate_span_attributes(data))
        span.status.MergeFrom(self._translate_status(data))
        span.resource.MergeFrom(self._translate_resource(data))
        span.telemetry_sdk.MergeFrom(self._translate_sdk(data))

        return span

    def _export_batch(self, timeout: Optional[int] = None) -> bool:
        """Export the spans to clearcut via http api."""

        spans = self._queue[:self._max_queue_size]
        self._queue = self._queue[self._max_queue_size:]

        wait_delta = self._next_request_dt - datetime.datetime.now()
        wait_time = wait_delta.total_seconds()

        # Drop the packets if wait time is more than threshold.
        if wait_time > self._max_wait_secs:
            logging.warning(
                'dropping %d spans for large wait: %d',
                len(spans),
                wait_time,
            )
            return True

        if wait_time > 0:
            time.sleep(wait_time)

        logrequest = self._prepare_request_body(spans)

        req = urllib.request.Request(
            self._endpoint,
            data=logrequest.SerializeToString(),
            method='POST',
        )
        logresponse = clientanalytics_pb2.LogResponse()

        try:
            with urllib.request.urlopen(req, timeout=timeout
                                        or self._timeout) as f:
                logresponse.ParseFromString(f.read())
        except urllib.error.URLError as url_exception:
            logging.warning(url_exception)
            return False
        except proto_msg.DecodeError as decode_error:
            logging.warning('could not decode data into proto: %s',
                            decode_error)
            return False

        now = datetime.datetime.now()
        delta = datetime.timedelta(
            milliseconds=logresponse.next_request_wait_millis)
        self._next_request_dt = now + delta
        return True

    def _prepare_request_body(self, spans) -> clientanalytics_pb2.LogRequest:
        log_request = clientanalytics_pb2.LogRequest()
        log_request.request_time_ms = int(time.time() * 1000)
        log_request.client_info.client_type = _CLIENT_TYPE
        log_request.log_source = self._log_source

        for span in spans:
            log_event = log_request.log_event.add()
            log_event.event_time_ms = int(time.time() * 1000)
            log_event.source_extension = span.SerializeToString()

        return log_request
