# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Test the trace implementation."""

import contextlib
from typing import Sequence

from opentelemetry import trace as trace_api
from opentelemetry.sdk import trace as trace_sdk
from opentelemetry.sdk import resources
from opentelemetry.sdk.trace import export as export_sdk

from . import trace as trace


class SpanExporterStub(export_sdk.SpanExporter):
    """Stub for SpanExporter."""

    def __init__(self) -> None:
        self._spans = []

    @property
    def spans(self):
        return self._spans

    def export(
            self, spans: Sequence[trace_sdk.ReadableSpan]
    ) -> export_sdk.SpanExportResult:
        self._spans.extend(spans)

        return export_sdk.SpanExportResult.SUCCESS


def test_span_to_capture_keyboard_interrupt_as_decorator() -> None:
    """Test span can capture KeyboardInterrupt as decorator."""
    exporter = SpanExporterStub()
    provider = trace.TracerProvider(trace_sdk.TracerProvider())
    provider.add_span_processor(
        export_sdk.BatchSpanProcessor(span_exporter=exporter))
    tracer = provider.get_tracer(__name__)

    @tracer.start_as_current_span("test-span")
    def test_function() -> None:
        raise KeyboardInterrupt()

    with contextlib.suppress(KeyboardInterrupt):
        test_function()

    provider.shutdown()

    assert len(exporter.spans) == 1
    assert exporter.spans[0].events[0].name == "exception"
    assert (exporter.spans[0].events[0].attributes["exception.type"] ==
            "KeyboardInterrupt")
    assert exporter.spans[0].status.status_code == trace_api.StatusCode.OK


def test_span_to_capture_keyboard_interrupt_in_context() -> None:
    """Test span can capture KeyboardInterrupt in context."""
    exporter = SpanExporterStub()
    provider = trace.TracerProvider(trace_sdk.TracerProvider())
    provider.add_span_processor(
        export_sdk.BatchSpanProcessor(span_exporter=exporter))
    tracer = provider.get_tracer(__name__)

    with contextlib.suppress(KeyboardInterrupt):
        with tracer.start_as_current_span("test-span"):
            raise KeyboardInterrupt()

    provider.shutdown()

    assert len(exporter.spans) == 1
    assert exporter.spans[0].events[0].name == "exception"
    assert (exporter.spans[0].events[0].attributes["exception.type"] ==
            "KeyboardInterrupt")
    assert exporter.spans[0].status.status_code == trace_api.StatusCode.OK
