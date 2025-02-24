# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Util for anonymizing telemetry spans."""

import getpass
import re

from typing import Optional, Pattern, Sequence, Tuple
from google.protobuf import json_format

from .proto import trace_span_pb2


class Anonymizer:
    """Redact the personally identifiable information."""

    def __init__(
        self,
        replacements: Optional[Sequence[Tuple[Pattern[str],
                                              str]]] = None) -> None:
        self._replacements = list(replacements or [])
        if getpass.getuser() != "root":
            # Substituting the root user doesn't actually anonymize anything.
            self._replacements.append(
                (re.compile(re.escape(getpass.getuser())), "<user>"))

    def __call__(self, *args, **kwargs):
        return self.apply(*args, **kwargs)

    def apply(self, data: str) -> str:
        """Applies the replacement rules to data text."""
        if not data:
            return data

        for repl_from, repl_to in self._replacements:
            data = re.sub(repl_from, repl_to, data)

        return data


class AnonymizingFilter:
    """Applies the anonymizer to TraceSpan messages."""

    def __init__(self, anonymizer: Anonymizer) -> None:
        self._anonymizer = anonymizer

    def __call__(self,
                 msg: trace_span_pb2.TraceSpan) -> trace_span_pb2.TraceSpan:
        """Applies the anonymizer to TraceSpan message."""
        raw = json_format.MessageToJson(msg)
        json_msg = self._anonymizer.apply(raw)
        output = trace_span_pb2.TraceSpan()
        json_format.Parse(json_msg, output)
        return output
