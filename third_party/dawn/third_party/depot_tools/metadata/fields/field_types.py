#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re
import sys
from typing import Optional
from enum import Enum

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
# The repo's root directory.
_ROOT_DIR = os.path.abspath(os.path.join(_THIS_DIR, "..", ".."))

# Add the repo's root directory for clearer imports.
sys.path.insert(0, _ROOT_DIR)

import metadata.fields.util as util
import metadata.validation_result as vr

# Pattern used to check if the entire string is either "yes" or "no",
# case-insensitive.
_PATTERN_YES_OR_NO = re.compile(r"^(yes|no)$", re.IGNORECASE)

# Pattern used to check if the string starts with "yes" or "no",
# case-insensitive. e.g. "No (test only)", "Yes?"
_PATTERN_STARTS_WITH_YES_OR_NO = re.compile(r"^(yes|no)", re.IGNORECASE)

class MetadataField:
    """Base class for all metadata fields."""

    # The delimiter used to separate multiple values.
    VALUE_DELIMITER = ","

    def __init__(self, name: str, structured: bool = True):
        self._name = name
        self._structured = structured

    def __eq__(self, other):
        if not isinstance(other, MetadataField):
            return False

        return self._name.lower() == other._name.lower()

    def __hash__(self):
        return hash(self._name.lower())

    def get_name(self):
        return self._name

    def should_terminate_field(self, field_value) -> bool:
        """Whether this field should be terminated based on the given `field_value`.
        """
        return False

    def is_structured(self):
        """Whether the field represents structured data, such as a list of
        URLs.

        If true, parser will treat `Field Name Like Pattern:` in subsequent
        lines as a new field, in addition to all known field names.
        If false, parser will only recognize known field names, and unknown
        fields will be merged into the preceding field value.
        """
        return self._structured

    def validate(self, value: str) -> Optional[vr.ValidationResult]:
        """Checks the given value is acceptable for the field.

        Raises: NotImplementedError if called. This method must be
                overridden with the actual validation of the field.
        """
        raise NotImplementedError(f"{self._name} field validation not defined.")

    def narrow_type(self, value):
        """Returns a narrowly typed (e.g. bool) value for this field for
           downstream consumption.

           The alternative being the downstream parses the string again.
        """
        raise NotImplementedError(
            f"{self._name} field value coersion not defined.")


class FreeformTextField(MetadataField):
    """Field where the value is freeform text."""

    def validate(self, value: str) -> Optional[vr.ValidationResult]:
        """Checks the given value has at least one non-whitespace
        character.
        """
        if util.is_empty(value):
            return vr.ValidationError(reason=f"{self._name} is empty.")

        return None

    def narrow_type(self, value):
        assert value is not None
        return value

class SingleLineTextField(FreeformTextField):
    """Field where the field as a whole is a single line of text."""

    def __init__(self, name):
        super().__init__(name=name)

    def should_terminate_field(self, field_value) -> bool:
        # Look for line breaks.
        #
        # We don't use `os.linesep`` here because Chromium uses line feed
        # (`\n`) for git checkouts on all platforms (see README.* entries
        # in chromium.src/.gitattributes).
        return field_value.endswith("\n")


class YesNoField(SingleLineTextField):
    """Field where the value must be yes or no."""
    def __init__(self, name: str):
        super().__init__(name=name)

    def validate(self, value: str) -> Optional[vr.ValidationResult]:
        """Checks the given value is either yes or no."""
        if util.matches(_PATTERN_YES_OR_NO, value):
            return None

        if util.matches(_PATTERN_STARTS_WITH_YES_OR_NO, value):
            return vr.ValidationWarning(
                reason=f"{self._name} is invalid.",
                additional=[
                    f"This field should be only {util.YES} or {util.NO}.",
                    f"Current value is '{value}'.",
                ])

        return vr.ValidationError(
            reason=f"{self._name} is invalid.",
            additional=[
                f"This field must be {util.YES} or {util.NO}.",
                f"Current value is '{value}'.",
            ])

    def narrow_type(self, value) -> Optional[bool]:
        return util.infer_as_boolean(super().narrow_type(value))
