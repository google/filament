# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Tracer, Provider and span context."""

import contextlib
from typing import Any, Iterator, Optional, Sequence
import logging
import pathlib
import sys

from opentelemetry import context as otel_context_api
from opentelemetry import trace as otel_trace_api
from opentelemetry.sdk import trace as otel_trace_sdk
from opentelemetry.util import types as otel_types

from . import config
from . import clearcut_span_exporter
from . import detector


@contextlib.contextmanager
def use_span(
    span: otel_trace_api.Span,
    end_on_exit: bool = False,
    record_exception: bool = True,
    set_status_on_exception: bool = True,
) -> Iterator[otel_trace_api.Span]:
    """Takes a non-active span and activates it in the current context."""

    try:
        token = otel_context_api.attach(
            # This is needed since the key needs to be the same as
            # used in the rest of opentelemetry code.
            otel_context_api.set_value(otel_trace_api._SPAN_KEY, span))
        try:
            yield span
        finally:
            otel_context_api.detach(token)

    except KeyboardInterrupt as exc:
        if span.is_recording():
            if record_exception:
                span.record_exception(exc)

            if set_status_on_exception:
                span.set_status(otel_trace_api.StatusCode.OK)
        raise
    except BaseException as exc:
        if span.is_recording():
            # Record the exception as an event
            if record_exception:
                span.record_exception(exc)

            # Set status in case exception was raised
            if set_status_on_exception:
                span.set_status(
                    otel_trace_api.Status(
                        status_code=otel_trace_api.StatusCode.ERROR,
                        description=f"{type(exc).__name__}: {exc}",
                    ))
        raise

    finally:
        if end_on_exit:
            span.end()


class Tracer(otel_trace_api.Tracer):
    """Specific otel tracer."""

    def __init__(self, inner: otel_trace_sdk.Tracer) -> None:
        self._inner = inner

    def start_span(
        self,
        name: str,
        context: Optional[otel_context_api.Context] = None,
        kind: otel_trace_api.SpanKind = otel_trace_api.SpanKind.INTERNAL,
        attributes: otel_types.Attributes = None,
        links: Optional[Sequence[otel_trace_api.Link]] = None,
        start_time: Optional[int] = None,
        record_exception: bool = True,
        set_status_on_exception: bool = True,
    ) -> otel_trace_api.Span:
        return self._inner.start_span(
            name,
            context=context,
            kind=kind,
            attributes=attributes,
            links=links,
            start_time=start_time,
            record_exception=record_exception,
            set_status_on_exception=set_status_on_exception,
        )

    @contextlib.contextmanager
    def start_as_current_span(
        self,
        name: str,
        context: Optional[otel_context_api.Context] = None,
        kind: otel_trace_api.SpanKind = otel_trace_api.SpanKind.INTERNAL,
        attributes: otel_types.Attributes = None,
        links: Optional[Sequence[otel_trace_api.Link]] = None,
        start_time: Optional[int] = None,
        record_exception: bool = True,
        set_status_on_exception: bool = True,
        end_on_exit: bool = True,
    ) -> Iterator[otel_trace_api.Span]:
        span = self.start_span(
            name=name,
            context=context,
            kind=kind,
            attributes=attributes,
            links=links,
            start_time=start_time,
            record_exception=record_exception,
            set_status_on_exception=set_status_on_exception,
        )
        with use_span(
                span,
                end_on_exit=end_on_exit,
                record_exception=record_exception,
                set_status_on_exception=set_status_on_exception,
        ) as span_context:
            yield span_context


class TracerProvider(otel_trace_api.TracerProvider):
    """Specific otel tracer provider."""

    def __init__(self, inner: otel_trace_sdk.TracerProvider) -> None:
        self._inner = inner

    def get_tracer(
        self,
        instrumenting_module_name: str,
        instrumenting_library_version: Optional[str] = None,
        schema_url: Optional[str] = None,
    ) -> otel_trace_api.Tracer:
        tracer = self._inner.get_tracer(
            instrumenting_module_name=instrumenting_module_name,
            instrumenting_library_version=instrumenting_library_version,
            schema_url=schema_url,
        )
        return Tracer(tracer)

    def __getattr__(self, name: str) -> Any:
        """Method allows to delegate method calls."""
        return getattr(self._inner, name)
