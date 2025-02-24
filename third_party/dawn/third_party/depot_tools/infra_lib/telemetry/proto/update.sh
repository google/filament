#!/usr/bin/env bash

gob-curl https://chromium.googlesource.com/chromiumos/chromite/+/main/api/gen_sdk/chromite/telemetry/clientanalytics_pb2.py?format=TEXT | base64 --decode > clientanalytics_pb2.py
gob-curl https://chromium.googlesource.com/chromiumos/chromite/+/main/api/gen_sdk/chromite/telemetry/trace_span_pb2.py?format=TEXT | base64 --decode > trace_span_pb2.py