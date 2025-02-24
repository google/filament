#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import datetime
import os
import sys
from typing import Optional, Tuple

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
# The repo's root directory.
_ROOT_DIR = os.path.abspath(os.path.join(_THIS_DIR, "..", "..", ".."))

# Add the repo's root directory for clearer imports.
sys.path.insert(0, _ROOT_DIR)

import metadata.fields.field_types as field_types
import metadata.validation_result as vr

# The preferred date format for the start of date values.
_PREFERRED_PREFIX_FORMAT = "%Y-%m-%d"

# Formats for the start of date values that are recognized as
# alternative date formats.
_RECOGNIZED_PREFIX_FORMATS = (
    "%d-%m-%Y",
    "%m-%d-%Y",
    "%d-%m-%y",
    "%m-%d-%y",
    "%d/%m/%Y",
    "%m/%d/%Y",
    "%d/%m/%y",
    "%m/%d/%y",
    "%d.%m.%Y",
    "%m.%d.%Y",
    "%d.%m.%y",
    "%m.%d.%y",
    "%Y/%m/%d",
    "%Y.%m.%d",
    "%Y%m%d",
)

# Formats recognized as alternative date formats (entire value must
# match).
_RECOGNIZED_DATE_FORMATS = (
    "%d %b %Y",
    "%d %b, %Y",
    "%b %d %Y",
    "%b %d, %Y",
    "%Y %b %d",
    "%d %B %Y",
    "%d %B, %Y",
    "%B %d %Y",
    "%B %d, %Y",
    "%Y %B %d",
    "%a %b %d %H:%M:%S %Y",
    "%a %b %d %H:%M:%S %Y %z",
)


def parse_with_format(value: str,
                      date_format: str) -> Optional[datetime.datetime]:
    """Returns datetime object if `value` can be parsed with `date_format`"""
    try:
        return datetime.datetime.strptime(value, date_format)
    except ValueError:
        return None


def to_preferred_format(dt: datetime.datetime) -> str:
    return datetime.datetime.strftime(dt, _PREFERRED_PREFIX_FORMAT)


def parse_date(value: str) -> Optional[Tuple[str, bool]]:
    """Try to parse value into a YYYY-MM-DD date.

       If successful: returns (str, int).
       - The str is guaranteed to be in YYYY-MM-DD format.
       - The bool indicates whether `value` is ambiguous.
         For example, "2020/03/05" matches both "YYYY/MM/DD" and "YYYY/DD/MM".
    """
    matches = []
    value = value.strip()
    if not value:
        return None

    first_part = value.split()[0]

    # Try to match preferred prefix.
    if dt := parse_with_format(first_part, _PREFERRED_PREFIX_FORMAT):
        matches.append(dt)

    if not matches:
        # Try alternative prefix formats.
        for date_format in _RECOGNIZED_PREFIX_FORMATS:
            if dt := parse_with_format(first_part, date_format):
                matches.append(dt)

    if not matches:
        # Try matching the complete string.
        for date_format in _RECOGNIZED_DATE_FORMATS:
            if dt := parse_with_format(value, date_format):
                matches.append(dt)

    if not matches:
        # Try ISO 8601.
        try:
            dt = datetime.datetime.fromisoformat(value)
            matches.append(dt)
        except ValueError:
            pass

    if not matches:
        return None

    # Determine if the value is parsed without ambiguity.
    is_ambiguous = len(set(map(to_preferred_format, matches))) > 1

    return to_preferred_format(matches[0]), is_ambiguous


class DateField(field_types.SingleLineTextField):
    """Custom field for the date when the package was updated."""
    def __init__(self):
        super().__init__(name="Date")

    def validate(self, value: str) -> Optional[vr.ValidationResult]:
        """Checks the given value is a YYYY-MM-DD date."""
        value = value.strip()
        if not value:
            return vr.ValidationError(
                reason=f"{self._name} is empty.",
                additional=["Provide date in format YYYY-MM-DD."])

        if not (parsed := parse_date(value)):
            return vr.ValidationError(
                reason=f"{self._name} is invalid.",
                additional=["Use YYYY-MM-DD.", f"Current value is '{value}'."])

        parsed_date, is_ambiguous = parsed
        if is_ambiguous:
            return vr.ValidationError(
                reason=f"{self._name} is ambiguous.",
                additional=["Use YYYY-MM-DD.", f"Current value is '{value}'."])

        if not parse_with_format(value, _PREFERRED_PREFIX_FORMAT):
            return vr.ValidationWarning(
                reason=f"{self._name} isn't using the canonical format.",
                additional=["Use YYYY-MM-DD.", f"Current value is '{value}'."])

        return None

    def narrow_type(self, value: str) -> Optional[str]:
        """Returns ISO 8601 date string, guarantees to be YYYY-MM-DD or None."""
        if not (parsed := parse_date(value)):
            return None

        # We still return a date even if the parsing result is ambiguous. An
        # date that's a few month off is better than nothing at all.
        return parsed[0]
