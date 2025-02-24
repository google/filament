# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from typing import Optional
import os
import socket
import sys
import pathlib

from opentelemetry import context as otel_context_api
from opentelemetry import trace as otel_trace_api
from opentelemetry.sdk import (
    resources as otel_resources,
    trace as otel_trace_sdk,
)
from opentelemetry.sdk.trace import export as otel_export
from opentelemetry.util import types as otel_types

from . import config
from . import clearcut_span_exporter
from . import detector

DEFAULT_BANNER = """
===============================================================================
To help improve the quality of this product, we collect usage data and
stacktraces from googlers. This includes a uuid generated weekly to identify
invocation from the same users as well as metrics as described in
go/chrome-infra-telemetry-readme. You may choose to opt out of this collection
at any time by setting the flag `enabled = False` under [trace] section in
{config_file}
or by executing from your depot_tools checkout:

vpython3 third_party/depot_tools/infra_lib/telemetry --disable

This notice will be displayed {run_count} more times.
===============================================================================
"""

# This does not include Googlers' physical machines/laptops
_GOOGLE_HOSTNAME_SUFFIX = ('.google.com', '.googler.com', '.googlers.com')

# The version keeps track of telemetry changes.
_TELEMETRY_VERSION = '3'


def get_host_name(fully_qualified: bool = False) -> str:
    """Return hostname of current machine, with domain if |fully_qualified|."""
    hostname = socket.gethostname()
    try:
        hostname = socket.gethostbyaddr(hostname)[0]
    except (socket.gaierror, socket.herror) as e:
        logging.warning(
            'please check your /etc/hosts file; resolving your hostname'
            ' (%s) failed: %s',
            hostname,
            e,
        )

    if fully_qualified:
        return hostname
    return hostname.partition('.')[0]


def is_google_host() -> bool:
    """Checks if the code is running on google host."""

    hostname = get_host_name(fully_qualified=True)
    return hostname.endswith(_GOOGLE_HOSTNAME_SUFFIX)


def initialize(service_name,
               notice=DEFAULT_BANNER,
               cfg_file=config.DEFAULT_CONFIG_FILE):
    if not is_google_host():
        return

    # TODO(326277821): Add support for other platforms
    if not sys.platform == 'linux':
        return

    cfg = config.Config(cfg_file)

    if not cfg.trace_config.has_enabled():
        if cfg.root_config.notice_countdown > -1:
            print(notice.format(run_count=cfg.root_config.notice_countdown,
                                config_file=cfg_file),
                  file=sys.stderr)
            cfg.root_config.update(
                notice_countdown=cfg.root_config.notice_countdown - 1)
        else:
            cfg.trace_config.update(enabled=True, reason='AUTO')

        cfg.flush()

    if not cfg.trace_config.enabled:
        return

    default_resource = otel_resources.Resource.create({
        otel_resources.SERVICE_NAME:
        service_name,
        'telemetry.version':
        _TELEMETRY_VERSION,
    })

    detected_resource = otel_resources.get_aggregated_resources([
        otel_resources.ProcessResourceDetector(),
        otel_resources.OTELResourceDetector(),
        detector.ProcessDetector(),
        detector.SystemDetector(),
    ])

    resource = detected_resource.merge(default_resource)
    trace_provider = otel_trace_sdk.TracerProvider(resource=resource)
    otel_trace_api.set_tracer_provider(trace_provider)
    trace_provider.add_span_processor(
        otel_export.BatchSpanProcessor(
            # Replace with ConsoleSpanExporter() to debug spans on the console
            clearcut_span_exporter.ClearcutSpanExporter()))


def get_tracer(name: str, version: Optional[str] = None):
    return otel_trace_api.get_tracer(name, version)
