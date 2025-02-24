#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re
from typing import List

# Preferred values for yes/no fields (i.e. all lowercase).
YES = "yes"
NO = "no"

# Pattern used to check if the entire string is "unknown",
# case-insensitive.
_PATTERN_UNKNOWN = re.compile(r"^unknown$", re.IGNORECASE)

# Pattern used to check if the entire string is functionally empty, i.e.
# empty string, or all characters are only whitespace.
_PATTERN_ONLY_WHITESPACE = re.compile(r"^\s*$")

# Pattern used to check if the string starts with "yes",
# case-insensitive.
_PATTERN_STARTS_WITH_YES = re.compile(r"^yes", re.IGNORECASE)

# Pattern used to check if the string starts with "no",
# case-insensitive.
_PATTERN_STARTS_WITH_NO = re.compile(r"^no", re.IGNORECASE)

# Variants of N/A (Not Applicable).
_PATTERN_NOT_APPLICABLE = re.compile(r"^(N ?\/ ?A)\.?|na\.?|not applicable\.?$",
                                     re.IGNORECASE)

# A collection of values that provides little information.
# Use lower-case for easier comparison.
_KNOWN_INVALID_VALUES = {
    "0",
    "varies",
    "-",
    "unknown",
    "head",
    "see deps",
    "deps",
}


def matches(pattern: re.Pattern, value: str) -> bool:
    """Returns whether the value matches the pattern."""
    return pattern.match(value) is not None


def is_empty(value: str) -> bool:
    """Returns whether the value is functionally empty."""
    return matches(_PATTERN_ONLY_WHITESPACE, value)


def is_unknown(value: str) -> bool:
    """Returns whether the value is 'unknown' (case insensitive)."""
    return matches(_PATTERN_UNKNOWN, value)


def quoted(values: List[str]) -> str:
    """Returns a string of the given values, each being individually
    quoted.
    """
    return ", ".join([f"'{entry}'" for entry in values])


def infer_as_boolean(value: str, default: bool = True) -> bool:
    """Attempts to infer the value as a boolean, where:
        - "yes"-ish values return True;
        - "no"-ish values return False; and
        - default is returned otherwise.
    """
    if matches(_PATTERN_STARTS_WITH_YES, value):
        return True
    elif matches(_PATTERN_STARTS_WITH_NO, value):
        return False
    else:
        return default


def is_known_invalid_value(value: str):
    """Returns whether `value` is among the known bad values that provides
       little machine readable information.
    """
    if not value:
        return False

    if value.lower() in _KNOWN_INVALID_VALUES:
        return True

    return False


def is_not_applicable(value: str) -> bool:
    return matches(_PATTERN_NOT_APPLICABLE, value)
