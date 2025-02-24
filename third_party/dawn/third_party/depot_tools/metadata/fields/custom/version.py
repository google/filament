#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re
import sys
from typing import Optional

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
# The repo's root directory.
_ROOT_DIR = os.path.abspath(os.path.join(_THIS_DIR, "..", "..", ".."))

# Add the repo's root directory for clearer imports.
sys.path.insert(0, _ROOT_DIR)

import metadata.fields.field_types as field_types
import metadata.fields.util as util
import metadata.validation_result as vr


def version_is_unknown(value: str) -> bool:
    """Returns whether the value denotes the version being unknown."""
    return (value == "0" or util.is_not_applicable(value)
            or util.is_unknown(value))


class VersionField(field_types.SingleLineTextField):
    """Custom field for the package version."""

    def __init__(self):
        super().__init__(name="Version")

    def validate(self, value: str) -> Optional[vr.ValidationResult]:
        """Checks the given value is acceptable - there must be at least
        one non-whitespace character, and "N/A" is preferred over "0" if
        the version is unknown.
        """
        if value == "0" or util.is_unknown(value):
            return vr.ValidationWarning(
                reason=f"{self._name} is '{value}'.",
                additional=[
                    "Set this field to 'N/A' if this package does not version "
                    "or is versioned by date or revision.",
                ])

        if util.is_empty(value):
            return vr.ValidationError(
                reason=f"{self._name} is empty.",
                additional=[
                    "Set this field to 'N/A' if this package does not version "
                    "or is versioned by date or revision.",
                ])

        return None

    def narrow_type(self, value: str) -> Optional[str]:
        value = super().narrow_type(value)
        if not value:
            return None

        if version_is_unknown(value):
            return None

        if util.is_known_invalid_value(value):
            return None

        return value
