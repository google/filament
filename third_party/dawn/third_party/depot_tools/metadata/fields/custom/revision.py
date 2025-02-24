#!/usr/bin/env python3
# Copyright 2024 The Chromium Authors. All rights reserved.
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
import metadata.fields.custom.version as version_field
import metadata.fields.util as util
import metadata.validation_result as vr

HEX_PATTERN = re.compile(r"^[a-fA-F0-9]{7,40}$")

# A special pattern to indicate that revision is written in DEPS file.
DEPS_PATTERN = re.compile(r"^DEPS$")


class RevisionField(field_types.SingleLineTextField):
    """Custom field for the revision."""

    def __init__(self):
        super().__init__(name="Revision")

    def is_revision_in_deps(self, value: str) -> bool:
        return bool(DEPS_PATTERN.match(value))

    def narrow_type(self, value: str) -> Optional[str]:
        value = super().narrow_type(value)
        if not value:
            return None

        if version_field.version_is_unknown(value):
            return None

        if util.is_known_invalid_value(value):
            return None

        if not HEX_PATTERN.match(value):
            return None

        return value

    def validate(self, value: str) -> Optional[vr.ValidationResult]:
        """Validates the revision string.

        Checks:
          - Non-empty value.
          - Valid hexadecimal format (length 7-40 characters).
        """
        if self.is_revision_in_deps(value):
            return None

        if util.is_unknown(value):
            return vr.ValidationWarning(
                reason=f"{self._name} is invalid.",
                additional=[
                    "Revision is required for dependencies which have a git repository "
                    "as an upstream, OPTIONAL if the upstream is not a git repository "
                    "and either Version or Date is supplied.",
                    "'{value}' is not a valid commit hash.",
                ],
            )

        if not HEX_PATTERN.match(value):
            return vr.ValidationError(
                reason=f"{self._name} is not a valid hexadecimal revision.",
                additional=[
                    "Revisions must be hexadecimal strings with a length of 7 to 40 characters."
                ],
            )

        # Valid revision.
        return None
