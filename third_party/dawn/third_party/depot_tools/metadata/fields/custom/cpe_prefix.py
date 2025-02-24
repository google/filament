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

# Pattern that will match CPE 2.2 and CPE 2.3 URN format.
#
# Adapted from https://csrc.nist.gov/schema/cpe/2.3/cpe-naming_2.3.xsd.
# We added a capture group (i.e. `match.group(1)`) for components-list, so we
# can determine if the CPE prefix provides sufficient information (see
# `is_adequate_cpe_urn` below).
_PATTERN_CPE_URN = re.compile(
    r"^[c][pP][eE]:/[AHOaho]?((:[A-Za-z0-9\._\-~%]*){0,6})$")

# Pattern that will match CPE 2.3 string format.
# Taken from https://csrc.nist.gov/schema/cpe/2.3/cpe-naming_2.3.xsd
_PATTERN_CPE_FORMATTED_STRING = re.compile(
    r"^cpe:2\.3:[aho\*\-](:(((\?*|\*?)([a-zA-Z0-9\-\._]|(\\[\\\*\?!\"#$$%&'\(\)\+,/:;<=>@\[\]\^`\{\|}~]))+(\?*|\*?))|[\*\-])){5}(:(([a-zA-Z]{2,3}(-([a-zA-Z]{2}|[0-9]{3}))?)|[\*\-]))(:(((\?*|\*?)([a-zA-Z0-9\-\._]|(\\[\\\*\?!\"#$$%&'\(\)\+,/:;<=>@\[\]\^`\{\|}~]))+(\?*|\*?))|[\*\-])){4}$"
)


def is_adequate_cpe_urn(value) -> bool:
    """Returns whether a given `value` conforms to CPE 2.3 and CPE 2.2 URN
    format, and the given `value` provides at least one component other than
    "part".

    See CPE Naming Specification: https://csrc.nist.gov/pubs/ir/7695/final.
    """
    m = _PATTERN_CPE_URN.match(value)
    if not m:
        # Didn't match CPE URN format.
        return False

    cpe_components = m.group(1).split(':')
    non_empty_components = list(filter(len, cpe_components))
    return len(non_empty_components) > 0


class CPEPrefixField(field_types.SingleLineTextField):
    """Custom field for the package's CPE."""
    def __init__(self):
        super().__init__(name="CPEPrefix")

    def _is_valid(self, value: str) -> bool:
        return util.is_unknown(value) or is_adequate_cpe_urn(
            value) or util.matches(_PATTERN_CPE_FORMATTED_STRING, value)

    def validate(self, value: str) -> Optional[vr.ValidationResult]:
        """Checks the given value is either 'unknown', or conforms to
        either the CPE 2.3 or 2.2 format.
        """
        if self._is_valid(value):
            return None

        return vr.ValidationError(
            reason=f"{self._name} is invalid.",
            additional=[
                "This field should be a CPE (version 2.3 or 2.2), "
                "or 'unknown'.",
                "Search for a CPE tag for the package at "
                "https://nvd.nist.gov/products/cpe/search.",
                "Please provide at least one CPE component other than part "
                "(e.g. a for Application, h for Hardware, o for Operating "
                "System)."
                f"Current value: '{value}'.",
            ])

    def narrow_type(self, value: str) -> Optional[str]:
        if not self._is_valid(value):
            return None

        if util.is_unknown(value):
            return None

        # CPE names are case-insensitive, we normalize to lowercase.
        # See https://cpe.mitre.org/specification/.
        value = value.lower()

        return value
