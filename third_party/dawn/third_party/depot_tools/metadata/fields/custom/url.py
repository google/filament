#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re
import sys
from typing import Optional, List
from urllib.parse import urlparse, urlunparse
from itertools import filterfalse

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
# The repo's root directory.
_ROOT_DIR = os.path.abspath(os.path.join(_THIS_DIR, "..", "..", ".."))

# Add the repo's root directory for clearer imports.
sys.path.insert(0, _ROOT_DIR)

import metadata.fields.field_types as field_types
import metadata.fields.util as util
import metadata.validation_result as vr

_PATTERN_URL_CANONICAL_REPO = re.compile(
    r"^This is the canonical (public )?repo(sitory)?\.?$", re.IGNORECASE)

_SUPPORTED_SCHEMES = {
    'http',
    'https',
    'git',
    'ftp',
}

# URLs can't contain whitespaces. Treat them as delimiters so we can handle cases where URL field contains one URL per line (without comma delimiter).
_PATTERN_URL_DELIMITER = re.compile("{}|{}".format(
    r'\s+', field_types.MetadataField.VALUE_DELIMITER))


def _split_urls(value: str) -> List[str]:
    """Split url field value into individual URLs."""
    urls = _PATTERN_URL_DELIMITER.split(value)
    return list(filter(lambda x: len(x) > 0, map(str.strip, urls)))


def _url_canonicalize(url: str) -> str:
    """Return the canonicalized URL (e.g. make scheme lower case)."""
    return urlunparse(urlparse(url))


def _url_is_canonical(url: str) -> bool:
    return url == _url_canonicalize(url)


def _url_is_valid(url: str) -> bool:
    """Checks whether the given `url` is acceptable:
       * url is can be parsed without an error.
       * url uses a supported scheme / protocol.
    """
    try:
        u = urlparse(url)
    except:
        return False

    if u.scheme not in _SUPPORTED_SCHEMES:
        return False

    return True

class URLField(field_types.MetadataField):
    """Custom field for the package URL(s)."""
    def __init__(self):
        super().__init__(name="URL")

    def repo_is_canonical(self, value: str):
        """Returns if `raw_value` indicates this repository is the canonical repository."""
        return util.matches(_PATTERN_URL_CANONICAL_REPO, value.strip())

    def validate(self, value: str) -> Optional[vr.ValidationResult]:
        """Checks the given value has acceptable URL values only.

        Note: this field supports multiple values.
        """
        if self.repo_is_canonical(value):
            return None

        urls = _split_urls(value)
        if not urls:
            return vr.ValidationError(reason=f"{self._name} must be provided.")

        invalid_values = list(filterfalse(_url_is_valid, urls))

        if invalid_values:
            return vr.ValidationError(
                reason=f"{self._name} is invalid.",
                additional=[
                    "URLs must use a protocol scheme in "
                    "[http, https, ftp, git].",
                    f"Separate URLs using a '{self.VALUE_DELIMITER}'.",
                    f"Invalid values: {util.quoted(invalid_values)}.",
                ])

        non_canon_values = list(filterfalse(_url_is_canonical, urls))
        if non_canon_values:
            canon_values = list(map(_url_canonicalize, non_canon_values))
            return vr.ValidationWarning(
                reason=f"{self._name} is contains non-canonical URLs.",
                additional=[
                    "URLs should be canonical and well-formed."
                    f"Non canonical values: {util.quoted(non_canon_values)}.",
                    f"Canonicalized URLs should be: {util.quoted(canon_values)}."
                ])

        return None

    def narrow_type(self, value) -> Optional[List[str]]:
        if not value:
            return None

        if self.repo_is_canonical(value):
            return None

        # Filter out invalid URLs, and canonicalize the URLs.
        return list(
            map(_url_canonicalize, filter(_url_is_valid, _split_urls(value))))
