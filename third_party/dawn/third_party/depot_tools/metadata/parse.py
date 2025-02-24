#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re
import sys
from typing import List

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
# The repo's root directory.
_ROOT_DIR = os.path.abspath(os.path.join(_THIS_DIR, ".."))

# Add the repo's root directory for clearer imports.
sys.path.insert(0, _ROOT_DIR)

import metadata.fields.known as known_fields
import metadata.dependency_metadata as dm

# Line used to separate dependencies within the same metadata file.
DEPENDENCY_DIVIDER = re.compile(r"^-{20} DEPENDENCY DIVIDER -{20}$")

# Delimiter used to separate a field's name from its value.
FIELD_DELIMITER = ":"

# Heuristic for detecting unknown field names.
_PATTERN_FIELD_NAME_WORD_HEURISTIC = r"[A-Z]\w+"
_PATTERN_FIELD_NAME_HEURISTIC = re.compile(r"^({}(?: {})*){}[\b\s]".format(
    _PATTERN_FIELD_NAME_WORD_HEURISTIC, _PATTERN_FIELD_NAME_WORD_HEURISTIC,
    FIELD_DELIMITER))
_DEFAULT_TO_STRUCTURED_TEXT = False

# Pattern used to check if a line from a metadata file declares a new
# field.
_PATTERN_KNOWN_FIELD_DECLARATION = re.compile(
    "^({}){}".format("|".join(known_fields.ALL_FIELD_NAMES), FIELD_DELIMITER),
    re.IGNORECASE)


def parse_content(content: str) -> List[dm.DependencyMetadata]:
    """Reads and parses the metadata from the given string.

    Args:
        content: the string to parse metadata from.

    Returns: all the metadata, which may be for zero or more
             dependencies, from the given string.
  """
    dependencies = []
    current_metadata = dm.DependencyMetadata()
    current_field_spec = None
    current_field_name = None
    current_field_value = ""

    for line_number, line in enumerate(content.splitlines(keepends=True), 1):
        # Whether the current line should be part of a structured value.
        if current_field_spec:
            expect_structured_field_value = current_field_spec.is_structured()
        else:
            expect_structured_field_value = _DEFAULT_TO_STRUCTURED_TEXT

        # Check if a new dependency is being described.
        if DEPENDENCY_DIVIDER.match(line):
            if current_field_name:
                # Save the field value for the previous dependency.
                current_metadata.add_entry(current_field_name,
                                           current_field_value)
            if current_metadata.has_entries():
                # Add the previous dependency to the results.
                dependencies.append(current_metadata)

            # Reset for the new dependency's metadata,
            # and reset the field state.
            current_metadata = dm.DependencyMetadata()
            current_field_spec = None
            current_field_name = None
            current_field_value = ""

        elif _PATTERN_KNOWN_FIELD_DECLARATION.match(line) or (
                expect_structured_field_value
                and _PATTERN_FIELD_NAME_HEURISTIC.match(line)):
            # Save the field value to the current dependency's metadata.
            if current_field_name:
                current_metadata.add_entry(current_field_name,
                                           current_field_value)

            current_field_name, current_field_value = line.split(
                FIELD_DELIMITER, 1)
            current_field_spec = known_fields.get_field(current_field_name)

            current_metadata.record_line(line_number)
            if current_field_spec:
                current_metadata.record_field_line_number(
                    current_field_spec, line_number)

        elif current_field_name:
            if line.strip():
                current_metadata.record_line(line_number)
            if current_field_spec:
                current_metadata.record_field_line_number(
                    current_field_spec, line_number)
            # The field is on multiple lines, so add this line to the
            # field value.
            current_field_value += line

        else:
            # Text that aren't part of any field (e.g. free form text).
            # Record the line number if the line is non-empty.
            if line.strip():
                current_metadata.record_line(line_number)

        # Check if current field value indicates end of the field.
        if current_field_spec and current_field_spec.should_terminate_field(
                current_field_value):
            assert current_field_name
            current_metadata.record_line(line_number)
            if current_field_spec:
                current_metadata.record_field_line_number(
                    current_field_spec, line_number)
            current_metadata.add_entry(current_field_name, current_field_value)
            current_field_spec = None
            current_field_name = None
            current_field_value = ""

    # At this point, the end of the file has been reached.
    # Save any remaining field data and metadata.
    if current_field_name:
        current_metadata.add_entry(current_field_name, current_field_value)
    if current_metadata.has_entries():
        dependencies.append(current_metadata)

    return dependencies
