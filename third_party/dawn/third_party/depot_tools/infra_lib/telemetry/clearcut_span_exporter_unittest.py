# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unittests for SpanExporter classes."""

import datetime
import re
import time
import urllib.request

from opentelemetry.sdk import trace
from opentelemetry.sdk.trace import export

from .proto import clientanalytics_pb2
from .proto import trace_span_pb2
from . import anonymization
from . import clearcut_span_exporter


class MockResponse:
    """Mock requests.Response."""

    def __init__(self, status, text) -> None:
        self._status = status
        self._text = text

    def __enter__(self):
        return self

    def __exit__(self, *args) -> None:
        pass

    def read(self):
        return self._text


tracer = trace.TracerProvider().get_tracer(__name__)


def test_otel_span_translation(monkeypatch) -> None:
    """Test ClearcutSpanExporter to translate otel spans to TraceSpan."""
    requests = []

    def mock_urlopen(request, timeout=0):
        requests.append((request, timeout))
        resp = clientanalytics_pb2.LogResponse()
        resp.next_request_wait_millis = 1
        body = resp.SerializeToString()
        return MockResponse(200, body)

    monkeypatch.setattr(urllib.request, "urlopen", mock_urlopen)

    span = tracer.start_span("name")
    span.end()

    e = clearcut_span_exporter.ClearcutSpanExporter(max_queue_size=1)

    assert e.export([span]) == export.SpanExportResult.SUCCESS
    req, _ = requests[0]
    log_request = clientanalytics_pb2.LogRequest()
    log_request.ParseFromString(req.data)

    assert log_request.request_time_ms <= int(time.time() * 1000)
    assert len(log_request.log_event) == 1

    # The following constants are defined in clearcut_span_exporter
    # as _CLIENT_TYPE and _LOG_SOURCE respectively.
    assert log_request.client_info.client_type == 33
    assert log_request.log_source == 2044

    tspan = trace_span_pb2.TraceSpan()
    tspan.ParseFromString(log_request.log_event[0].source_extension)

    assert tspan.name == span.name
    assert tspan.start_time_millis == int(span.start_time / 1e6)
    assert tspan.end_time_millis == int(span.end_time / 1e6)


def test_otel_span_translation_with_anonymization(monkeypatch) -> None:
    """Test ClearcutSpanExporter to anonymize spans to before export."""
    requests = []

    def mock_urlopen(request, timeout=0):
        requests.append((request, timeout))
        resp = clientanalytics_pb2.LogResponse()
        resp.next_request_wait_millis = 1
        body = resp.SerializeToString()
        return MockResponse(200, body)

    monkeypatch.setattr(urllib.request, "urlopen", mock_urlopen)

    span = tracer.start_span("span-user4321")
    span.set_attributes({"username": "user4321"})
    span.add_event("event-for-user4321")
    span.end()

    anonymizer = anonymization.Anonymizer([(re.escape("user4321"), "<user>")])
    f = anonymization.AnonymizingFilter(anonymizer)
    e = clearcut_span_exporter.ClearcutSpanExporter(prefilter=f,
                                                    max_queue_size=1)

    assert e.export([span]) == export.SpanExportResult.SUCCESS
    req, _ = requests[0]
    log_request = clientanalytics_pb2.LogRequest()
    log_request.ParseFromString(req.data)

    tspan = trace_span_pb2.TraceSpan()
    tspan.ParseFromString(log_request.log_event[0].source_extension)

    assert tspan.name == "span-<user>"
    assert tspan.events[0].name == "event-for-<user>"
    assert tspan.attributes["username"] == "<user>"


def test_export_to_http_api(monkeypatch) -> None:
    """Test ClearcutSpanExporter to export spans over http."""
    requests = []

    def mock_urlopen(request, timeout=0):
        requests.append((request, timeout))
        resp = clientanalytics_pb2.LogResponse()
        resp.next_request_wait_millis = 1
        body = resp.SerializeToString()
        return MockResponse(200, body)

    monkeypatch.setattr(urllib.request, "urlopen", mock_urlopen)

    span = tracer.start_span("name")
    span.end()
    endpoint = "http://domain.com/path"

    e = clearcut_span_exporter.ClearcutSpanExporter(endpoint=endpoint,
                                                    timeout=7,
                                                    max_queue_size=1)

    assert e.export([span])
    req, timeout = requests[0]
    assert req.full_url == endpoint
    assert timeout == 7


def test_export_to_http_api_throttle(monkeypatch) -> None:
    """Test ClearcutSpanExporter to throttle based on prev response."""
    mock_open_times = []

    def mock_urlopen(request, timeout=0):
        mock_open_times.append(datetime.datetime.now())
        resp = clientanalytics_pb2.LogResponse()
        resp.next_request_wait_millis = 1000
        body = resp.SerializeToString()
        return MockResponse(200, body)

    monkeypatch.setattr(urllib.request, "urlopen", mock_urlopen)

    span = tracer.start_span("name")
    span.end()

    e = clearcut_span_exporter.ClearcutSpanExporter(max_queue_size=1)

    assert e.export([span])
    assert e.export([span])

    # We've called export() on the same exporter instance twice, so we expect
    # the following things to be true:
    #   1. The request.urlopen() function has been called exactly twice, and
    #   2. The calls to urlopen() are more than 1000 ms apart (due to the
    #      value in the mock_urlopen response).
    # The mock_open_times list is a proxy for observing this behavior directly.
    assert len(mock_open_times) == 2
    assert (mock_open_times[1] - mock_open_times[0]).total_seconds() > 1


def test_export_to_drop_spans_if_wait_more_than_threshold(monkeypatch) -> None:
    """Test ClearcutSpanExporter to drop span if wait is more than threshold."""
    mock_open_times = []

    def mock_urlopen(request, timeout=0):
        nonlocal mock_open_times
        mock_open_times.append(datetime.datetime.now())
        resp = clientanalytics_pb2.LogResponse()
        resp.next_request_wait_millis = 900000
        body = resp.SerializeToString()
        return MockResponse(200, body)

    monkeypatch.setattr(urllib.request, "urlopen", mock_urlopen)

    span = tracer.start_span("name")
    span.end()

    e = clearcut_span_exporter.ClearcutSpanExporter(max_queue_size=1)

    assert e.export([span])
    assert e.export([span])

    # We've called export() on the same exporter instance twice, so we expect
    # the following things to be true:
    #   1. The request.urlopen() function has been called exactly once
    assert len(mock_open_times) == 1


def test_flush_to_clear_export_queue_to_http_api(monkeypatch) -> None:
    """Test ClearcutSpanExporter to export spans on flush."""
    requests = []

    def mock_urlopen(request, timeout=0):
        requests.append((request, timeout))
        resp = clientanalytics_pb2.LogResponse()
        resp.next_request_wait_millis = 1
        body = resp.SerializeToString()
        return MockResponse(200, body)

    monkeypatch.setattr(urllib.request, "urlopen", mock_urlopen)

    span = tracer.start_span("name")
    span.end()

    e = clearcut_span_exporter.ClearcutSpanExporter(max_queue_size=3)

    assert e.export([span])
    assert e.export([span])
    assert len(requests) == 0

    assert e.force_flush()
    assert len(requests) == 1


def test_shutdown_to_clear_export_queue_to_http_api(monkeypatch) -> None:
    """Test ClearcutSpanExporter to export spans on shutdown."""
    requests = []

    def mock_urlopen(request, timeout=0):
        requests.append((request, timeout))
        resp = clientanalytics_pb2.LogResponse()
        resp.next_request_wait_millis = 1
        body = resp.SerializeToString()
        return MockResponse(200, body)

    monkeypatch.setattr(urllib.request, "urlopen", mock_urlopen)

    span = tracer.start_span("name")
    span.end()

    e = clearcut_span_exporter.ClearcutSpanExporter(max_queue_size=3)

    assert e.export([span])
    assert e.export([span])
    assert len(requests) == 0

    e.shutdown()
    assert len(requests) == 1
