#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re
import sys
from typing import List, Tuple, Optional

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
# The repo's root directory.
_ROOT_DIR = os.path.abspath(os.path.join(_THIS_DIR, "..", "..", ".."))
# Bad delimiter characters.
BAD_DELIMITERS = ["/", ";", " and ", " or "]
BAD_DELIMITERS_REGEX = re.compile("|".join(re.escape(delimiter) for delimiter in BAD_DELIMITERS))

# Add the repo's root directory for clearer imports.
sys.path.insert(0, _ROOT_DIR)

import metadata.fields.field_types as field_types
import metadata.fields.util as util
import metadata.validation_result as vr
from metadata.fields.custom.license_allowlist import ALLOWED_LICENSES, ALLOWED_OPEN_SOURCE_LICENSES, ALL_LICENSES, WITH_PERMISSION_ONLY


def process_license_value(value: str,
                          atomic_delimiter: str) -> List[Tuple[str, bool]]:
    """Process a license field value, which may list multiple licenses.

    Args:
        value: the value to process, which may include both verbose and
               atomic delimiters, e.g. "Apache, 2.0 and MIT and custom"
        atomic_delimiter: the delimiter to use as a final step; values
                          will not be further split after using this
                          delimiter.

    Returns: a list of the constituent licenses within the given value,
             and whether the constituent license is a recognized license type.
             e.g. [("Apache, 2.0", True), ("MIT", True),
                   ("custom", False)]
    """
    # Check if the value is on the allowlist as-is, and thus does not
    # require further processing.
    if is_license_valid(value):
        return [(value, True)]

    breakdown = []
    # Split using the standard value delimiter. This results in
    # atomic values; there is no further splitting possible.
    for atomic_value in value.split(atomic_delimiter):
        atomic_value = atomic_value.strip()
        breakdown.append(
            (atomic_value, is_license_valid(
                atomic_value,
            ))
        )

    return breakdown


def is_license_valid(value: str) -> bool:
    """Returns whether the value is a valid license type.
    """
    return value in ALL_LICENSES

def is_license_allowlisted(value: str, is_open_source_project: bool = False) -> bool:
    """Returns whether the value is in the allowlist for license
    types.
    """
    # Restricted licenses are not enforced by presubmits, see b/388620886 😢.
    if value in WITH_PERMISSION_ONLY:
      return True
    if is_open_source_project:
        return value in ALLOWED_OPEN_SOURCE_LICENSES
    return value in ALLOWED_LICENSES

class LicenseField(field_types.SingleLineTextField):
  """Custom field for the package's license type(s).

    e.g. Apache-2.0, MIT, BSD-2.0
    """
  def __init__(self):
    super().__init__(name="License")

  def validate(self, value: str) -> Optional[vr.ValidationResult]:
    """Checks the given value consists of recognized license types.

        Note: this field supports multiple values.
        """
    not_allowlisted = []
    licenses = process_license_value(value,
          atomic_delimiter=self.VALUE_DELIMITER,
    )
    for license, allowed in licenses:
      if util.is_empty(license):
        return vr.ValidationError(
                    reason=f"{self._name} has an empty value.")
      if BAD_DELIMITERS_REGEX.search(license):
        return vr.ValidationError(
                reason=f"{self._name} contains a bad license separator. "
                "Separate licenses by commas only.",
                # Try and preemptively address the root cause of this behaviour,
                # which is having multiple choices for a license.
                additional=[f"When given a choice of licenses, chose the most "
                            "permissive one, do not list all options."]
        )
      if not allowed:
        not_allowlisted.append(license)

    if not_allowlisted:
      return vr.ValidationWarning(
                reason=f"{self._name} has a license not in the allowlist."
                " (see https://source.chromium.org/chromium/chromium/tools/depot_tools/+/main:metadata/fields/custom/license_allowlist.py).",
                additional=[
                    "Licenses not allowlisted: "
                    f"{util.quoted(not_allowlisted)}.",
                ])

    return None

  def narrow_type(self, value: str) -> Optional[List[str]]:
    if not value:
      # Empty License field is equivalent to "not declared".
      return None

    parts = value.split(self.VALUE_DELIMITER)
    return list(filter(bool, map(lambda str: str.strip(), parts)))
