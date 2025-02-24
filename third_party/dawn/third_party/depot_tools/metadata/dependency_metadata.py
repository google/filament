#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from collections import defaultdict
import os
import sys
import itertools
from typing import Dict, List, Set, Tuple, Union, Optional, Literal, Any

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
# The repo's root directory.
_ROOT_DIR = os.path.abspath(os.path.join(_THIS_DIR, ".."))

# Add the repo's root directory for clearer imports.
sys.path.insert(0, _ROOT_DIR)

import metadata.fields.field_types as field_types
import metadata.fields.custom.license as license_util
import metadata.fields.custom.version as version_util
import metadata.fields.known as known_fields
import metadata.fields.util as util
import metadata.validation_result as vr
from metadata.fields.custom.license_allowlist import OPEN_SOURCE_SPDX_LICENSES


class DependencyMetadata:
    """The metadata for a single dependency.

       See @property declarations below to retrieve validated fields for
       downstream consumption.

       The property returns `None` if the provided value (e.g. in
       README.chromium file) is clearly invalid.

       Otherwise, it returns a suitably typed value (see comments on each
       property).

       To retrieve unvalidated (i.e. raw values) fields, use get_entries().
    """

    # Fields that are always required.
    _MANDATORY_FIELDS = {
        known_fields.NAME,
        known_fields.URL,
        known_fields.VERSION,
        known_fields.LICENSE,
        known_fields.SECURITY_CRITICAL,
        known_fields.SHIPPED,
    }

    # Aliases for fields, where:
    #     * key is the alias field; and
    #     * value is the main field to which it should be mapped.
    # Note: if both the alias and main fields are specified in metadata,
    #       the value from the alias field will be used.
    _FIELD_ALIASES = {
        known_fields.SHIPPED_IN_CHROMIUM: known_fields.SHIPPED,
    }

    def __init__(self):
        # The record of all entries added, including repeated fields.
        self._entries: List[Tuple[str, str]] = []

        # The current value of each field.
        self._metadata: Dict[field_types.MetadataField, str] = {}

        # The line numbers of each metadata fields.
        self._metadata_line_numbers: Dict[field_types.MetadataField,
                                          Set[int]] = defaultdict(lambda: set())

        # The line numbers of the first and the last line (in the text file)
        # of this dependency metadata.
        self._first_line = float('inf')
        self._last_line = -1

        # The record of how many times a field entry was added.
        self._occurrences: Dict[field_types.MetadataField,
                                int] = defaultdict(int)

    def add_entry(self, field_name: str, field_value: str):
        value = field_value.strip()
        self._entries.append((field_name, value))

        field = known_fields.get_field(field_name)
        if field:
            self._metadata[field] = value
            self._occurrences[field] += 1

    def has_entries(self) -> bool:
        return len(self._entries) > 0

    def get_entries(self) -> List[Tuple[str, str]]:
        return list(self._entries)

    def record_line(self, line_number):
        """Records `line_number` to be part of this metadata."""
        self._first_line = min(self._first_line, line_number)
        self._last_line = max(self._last_line, line_number)

    def record_field_line_number(self, field: field_types.MetadataField,
                                 line_number: int):
        self._metadata_line_numbers[field].add(line_number)

    def get_first_and_last_line_number(self) -> Tuple[int, int]:
        return (self._first_line, self._last_line)

    def get_field_line_numbers(self,
                               field: field_types.MetadataField) -> List[int]:
        return sorted(self._metadata_line_numbers[field])

    def all_licenses_allowlisted(self, license_field_value: str, is_open_source_project: bool) -> bool:
        """Returns whether all licenses in the field are allowlisted.
        Assumes a non-empty license_field_value"""
        licenses = license_util.process_license_value(
            license_field_value,
            atomic_delimiter=known_fields.LICENSE.VALUE_DELIMITER)
        for lic, valid in licenses:
            allowed = license_util.is_license_allowlisted(lic, is_open_source_project=is_open_source_project)
            if not valid or not allowed:
                return False
        return True


    def only_open_source_licenses(self, license_field_value: str) ->List[str]:
        """Returns a list of licenses that are only allowed in open source projects."""
        licenses = license_util.process_license_value(
            license_field_value,
            atomic_delimiter=known_fields.LICENSE.VALUE_DELIMITER)
        open_source_only = []
        for lic, valid in licenses:
            if valid and lic in OPEN_SOURCE_SPDX_LICENSES:
                open_source_only.append(lic)
        return open_source_only

    def _assess_required_fields(self, is_open_source_project: bool = False) -> Set[field_types.MetadataField]:
        """Returns the set of required fields, based on the current
        metadata.
        """
        required = set(self._MANDATORY_FIELDS)

        # Assume the dependency is shipped if not specified.
        shipped_value = self._metadata.get(known_fields.SHIPPED)
        is_shipped = (shipped_value is None
                      or util.infer_as_boolean(shipped_value, default=True))

        if is_shipped:
            # A license file is required if the dependency is shipped.
            required.add(known_fields.LICENSE_FILE)

            # License compatibility with Android must be set if the
            # package is shipped and the license is not in the
            # allowlist.
            license_value = self._metadata.get(known_fields.LICENSE)
            if not license_value or not self.all_licenses_allowlisted(license_value, is_open_source_project):
                required.add(known_fields.LICENSE_ANDROID_COMPATIBLE)

        return required

    def validate(self, source_file_dir: str,
                 repo_root_dir: str,
                 is_open_source_project: bool = False) -> List[vr.ValidationResult]:
        """Validates all the metadata.

        Args:
            source_file_dir: the directory of the file that the metadata
                             is from.
            repo_root_dir: the repository's root directory.
            is_open_source_project: whether the project is open source.

        Returns: the metadata's validation results.
        """
        results = []

        # Check for duplicate fields.
        repeated_fields = [
            field for field, count in self._occurrences.items() if count > 1
        ]
        if repeated_fields:
            repeated = ", ".join([
                f"{field.get_name()} ({self._occurrences[field]})"
                for field in repeated_fields
            ])
            error = vr.ValidationError(reason="There is a repeated field.",
                                       additional=[
                                           f"Repeated fields: {repeated}",
                                       ])
            # Merge line numbers.
            lines = sorted(
                set(
                    itertools.chain.from_iterable([
                        self.get_field_line_numbers(field)
                        for field in repeated_fields
                    ])))
            error.set_lines(lines)
            results.append(error)

        # Process alias fields.
        sources = {}
        for alias_field, main_field in self._FIELD_ALIASES.items():
            if alias_field in self._metadata:
                # Validate the value that was present for the main field
                # before overwriting it with the alias field value.
                if main_field in self._metadata:
                    main_value = self._metadata.get(main_field)
                    field_result = main_field.validate(main_value)
                    if field_result:
                        field_result.set_tag(tag="field",
                                             value=main_field.get_name())
                        field_result.set_lines(
                            self.get_field_line_numbers(main_field))
                        results.append(field_result)

                self._metadata[main_field] = self._metadata[alias_field]
                sources[main_field] = alias_field
                self._metadata.pop(alias_field)

        # Validate values for all present fields.
        for field, value in self._metadata.items():
            source_field = sources.get(field) or field
            field_result = source_field.validate(value)
            if field_result:
                field_result.set_tag(tag="field", value=source_field.get_name())
                field_result.set_lines(
                    self.get_field_line_numbers(source_field))
                results.append(field_result)

        # Check required fields are present.
        required_fields = self._assess_required_fields(is_open_source_project=is_open_source_project)
        for field in required_fields:
            if field not in self._metadata:
                field_name = field.get_name()
                error = vr.ValidationError(
                    reason=f"Required field '{field_name}' is missing.")
                results.append(error)

        # If the repository is hosted somewhere (i.e. Chromium isn't the
        # canonical repositroy of the dependency), at least one of the fields
        # Version, Date or Revision must be provided.
        if (not (self.is_canonical or self.version or self.date or self.revision
                 or self.revision_in_deps)):
            versioning_fields = [
                known_fields.VERSION, known_fields.DATE, known_fields.REVISION
            ]
            names = util.quoted(
                [field.get_name() for field in versioning_fields])
            error = vr.ValidationError(
                reason="Versioning fields are insufficient.",
                additional=[f"Provide at least one of [{names}]."],
            )
            results.append(error)

        # Check existence of the license file(s) on disk.
        license_file_value = self._metadata.get(known_fields.LICENSE_FILE)
        if license_file_value is not None:
            result = known_fields.LICENSE_FILE.validate_on_disk(
                value=license_file_value,
                source_file_dir=source_file_dir,
                repo_root_dir=repo_root_dir,
            )
            if result:
                result.set_tag(tag="field",
                               value=known_fields.LICENSE_FILE.get_name())
                result.set_lines(
                    self.get_field_line_numbers(known_fields.LICENSE_FILE))
                results.append(result)


        if not is_open_source_project:
            license_value = self._metadata.get(known_fields.LICENSE)
            if license_value is not None:
                not_allowed_licenses = self.only_open_source_licenses(license_value)
                if len(not_allowed_licenses) > 0:
                    license_result = vr.ValidationWarning(
                        reason=f"License has a license not in the allowlist."
                        " (see https://source.chromium.org/chromium/chromium/tools/depot_tools/+/main:metadata/fields/custom/license_allowlist.py).",
                        additional=[
                            f"The following license{'s  are' if len(not_allowed_licenses) > 1 else ' is'} only allowed in open source projects: "
                            f"{util.quoted(not_allowed_licenses)}.",
                    ])

                    license_result.set_tag(tag="field", value=known_fields.LICENSE.get_name())
                    license_result.set_lines(
                        self.get_field_line_numbers(known_fields.LICENSE))
                    results.append(license_result)

        return results

    def _return_as_property(self, field: field_types.MetadataField) -> Any:
        """Helper function to create a property for DependencyMetadata.

        The property accessor will validate and return sanitized field value.
        """
        assert field in known_fields.ALL_FIELDS

        raw_value = self._metadata.get(field, None)
        if raw_value is None:
            # Field is not set.
            return None

        return field.narrow_type(raw_value)

    @property
    def name(self) -> Optional[str]:
        return self._return_as_property(known_fields.NAME)

    @property
    def short_name(self) -> Optional[str]:
        return self._return_as_property(known_fields.SHORT_NAME)

    @property
    def url(self) -> Optional[List[str]]:
        """
        Returns a list of URLs that points to upstream repo.
        The URLs are guaranteed to `urllib.parse.urlparse` without errors.

        Returns None if this repository is the canonical repository of this
        dependency (see is_canonical below).
        """
        return self._return_as_property(known_fields.URL)

    @property
    def is_canonical(self) -> bool:
        """
        Returns whether this repository is the canonical public repository of this dependency.

        This is derived from a special value in the URL field.
        """
        value = self._metadata.get(known_fields.URL, "")
        return known_fields.URL.repo_is_canonical(value)

    @property
    def version(self) -> Optional[str]:
        return self._return_as_property(known_fields.VERSION)

    @property
    def date(self) -> Optional[str]:
        """Returns in "YYYY-MM-DD" format."""
        return self._return_as_property(known_fields.DATE)

    @property
    def revision(self) -> Optional[str]:
        return self._return_as_property(known_fields.REVISION)

    @property
    def revision_in_deps(self) -> bool:
        value = self._metadata.get(known_fields.REVISION, "")
        return known_fields.REVISION.is_revision_in_deps(value)

    @property
    def license(self) -> Optional[List[str]]:
        """Returns a list of license names."""
        return self._return_as_property(known_fields.LICENSE)

    @property
    def license_file(self) -> Optional[List[str]]:
        # TODO(b/321154076): Consider excluding files that doesn't exist on
        # disk if it's not too hard.
        #
        # Plumbing src_root and dependency_dir into field validator is
        # required.
        return self._return_as_property(known_fields.LICENSE_FILE)

    @property
    def security_critical(self) -> Optional[bool]:
        return self._return_as_property(known_fields.SECURITY_CRITICAL)

    @property
    def shipped(self) -> Optional[bool]:
        return self._return_as_property(known_fields.SHIPPED)

    @property
    def shipped_in_chromium(self) -> Optional[bool]:
        return self._return_as_property(known_fields.SHIPPED_IN_CHROMIUM)

    @property
    def license_android_compatible(self) -> Optional[bool]:
        return self._return_as_property(known_fields.LICENSE_ANDROID_COMPATIBLE)

    @property
    def cpe_prefix(self) -> Optional[str]:
        """Returns a lowercase string (CPE names are case-insensitive)."""
        return self._return_as_property(known_fields.CPE_PREFIX)

    @property
    def description(self) -> Optional[str]:
        return self._return_as_property(known_fields.DESCRIPTION)

    @property
    def local_modifications(self) -> Optional[Union[Literal[False], str]]:
        """Returns `False` if there's no local modifications.
           Otherwise the text content extracted from the metadata.
        """
        return self._return_as_property(known_fields.LOCAL_MODIFICATIONS)
