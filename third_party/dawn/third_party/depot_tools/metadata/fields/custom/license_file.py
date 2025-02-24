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

# Pattern for backward directory navigation in paths.
_PATTERN_PATH_BACKWARD = re.compile(r"\.\.\/")

# Deprecated special value for packages that aren't shipped.
_NOT_SHIPPED = "NOT_SHIPPED"


class LicenseFileField(field_types.SingleLineTextField):
    """Custom field for the paths to the package's license file(s)."""
    def __init__(self):
        super().__init__(name="License File")

    def validate(self, value: str) -> Optional[vr.ValidationResult]:
        """Checks the given value consists of non-empty paths with no
        backward directory navigation (i.e. no "../").

        This validation is rudimentary. To check if the license file(s)
        exist on disk, see the `LicenseFileField.validate_on_disk`
        method.

        Note: this field supports multiple values.
        """
        if value == _NOT_SHIPPED:
            return vr.ValidationWarning(
                reason=f"{self._name} uses deprecated value '{_NOT_SHIPPED}'.",
                additional=[
                    f"Remove this field and use 'Shipped: {util.NO}' instead.",
                ])

        invalid_values = []
        for path in value.split(self.VALUE_DELIMITER):
            path = path.strip()
            if util.is_empty(path) or util.matches(_PATTERN_PATH_BACKWARD,
                                                   path):
                invalid_values.append(path)

        if invalid_values:
            return vr.ValidationError(
                reason=f"{self._name} is invalid.",
                additional=[
                    "File paths cannot be empty, or include '../'.",
                    "Separate license files using a "
                    f"'{self.VALUE_DELIMITER}'.",
                    f"Invalid values: {util.quoted(invalid_values)}.",
                ])

        return None

    def validate_on_disk(
        self,
        value: str,
        source_file_dir: str,
        repo_root_dir: str,
    ) -> Optional[vr.ValidationResult]:
        """Checks the given value consists of file paths which exist on
        disk.

        Note: this field supports multiple values.

        Args:
            value: the value to validate.
            source_file_dir: the directory of the metadata file that the
                             license file value is from; this is needed
                             to construct file paths to license files.
            repo_root_dir: the repository's root directory; this is
                           needed to construct file paths to
                           license files.

        Returns: a validation result based on the license file value,
                 and whether the license file(s) exist on disk,
                 otherwise None.
        """
        if value == _NOT_SHIPPED:
            return vr.ValidationWarning(
                reason=f"{self._name} uses deprecated value '{_NOT_SHIPPED}'.",
                additional=[
                    f"Remove this field and use 'Shipped: {util.NO}' instead.",
                ])

        invalid_values = []
        for license_filename in value.split(self.VALUE_DELIMITER):
            license_filename = license_filename.strip()
            if license_filename.startswith("/"):
                license_filepath = os.path.join(
                    repo_root_dir,
                    os.path.normpath(license_filename.lstrip("/")))
            else:
                license_filepath = os.path.join(
                    source_file_dir, os.path.normpath(license_filename))

            if not os.path.exists(license_filepath):
                rel_filepath = os.path.relpath(license_filepath, repo_root_dir)
                invalid_values.append(rel_filepath)

        if invalid_values:
            missing = ", ".join(invalid_values)
            return vr.ValidationError(
                reason=f"{self._name} is invalid.",
                additional=[
                    "Failed to find all license files on local disk.",
                    f"Missing files: {missing}.",
                ])

        return None
