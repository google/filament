#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
from typing import Optional

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
# The repo's root directory.
_ROOT_DIR = os.path.abspath(os.path.join(_THIS_DIR, "..", ".."))

# Add the repo's root directory for clearer imports.
sys.path.insert(0, _ROOT_DIR)

import metadata.fields.custom.cpe_prefix
import metadata.fields.custom.date
import metadata.fields.custom.license
import metadata.fields.custom.license_file
import metadata.fields.custom.local_modifications
import metadata.fields.custom.url
import metadata.fields.custom.version
import metadata.fields.custom.revision
import metadata.fields.field_types as field_types

# Freeform text fields.
NAME = field_types.SingleLineTextField("Name")
SHORT_NAME = field_types.SingleLineTextField("Short Name")
DESCRIPTION = field_types.FreeformTextField("Description", structured=False)

# Yes/no fields.
SECURITY_CRITICAL = field_types.YesNoField("Security Critical")
SHIPPED = field_types.YesNoField("Shipped")
SHIPPED_IN_CHROMIUM = field_types.YesNoField("Shipped in Chromium")
LICENSE_ANDROID_COMPATIBLE = field_types.YesNoField(
    "License Android Compatible")

# Custom fields.
CPE_PREFIX = metadata.fields.custom.cpe_prefix.CPEPrefixField()
DATE = metadata.fields.custom.date.DateField()
LICENSE = metadata.fields.custom.license.LicenseField()
LICENSE_FILE = metadata.fields.custom.license_file.LicenseFileField()
URL = metadata.fields.custom.url.URLField()
VERSION = metadata.fields.custom.version.VersionField()
REVISION = metadata.fields.custom.revision.RevisionField()
LOCAL_MODIFICATIONS = metadata.fields.custom.local_modifications.LocalModificationsField(
)

ALL_FIELDS = (
    NAME,
    SHORT_NAME,
    URL,
    VERSION,
    DATE,
    REVISION,
    LICENSE,
    LICENSE_FILE,
    SECURITY_CRITICAL,
    SHIPPED,
    SHIPPED_IN_CHROMIUM,
    LICENSE_ANDROID_COMPATIBLE,
    CPE_PREFIX,
    DESCRIPTION,
    LOCAL_MODIFICATIONS,
)
ALL_FIELD_NAMES = {field.get_name() for field in ALL_FIELDS}
_FIELD_MAPPING = {field.get_name().lower(): field for field in ALL_FIELDS}


def get_field(label: str) -> Optional[field_types.MetadataField]:
    return _FIELD_MAPPING.get(label.lower())
