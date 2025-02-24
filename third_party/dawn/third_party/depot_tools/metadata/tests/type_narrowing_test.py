#!/usr/bin/env python3
# Copyright 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
import unittest
from typing import Any, Callable

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
# The repo's root directory.
_ROOT_DIR = os.path.abspath(os.path.join(_THIS_DIR, "..", ".."))

# Add the repo's root directory for clearer imports.
sys.path.insert(0, _ROOT_DIR)

from metadata.fields.field_types import MetadataField
import metadata.fields.known as fields
from metadata.dependency_metadata import DependencyMetadata


class FieldValidationTest(unittest.TestCase):
    """Tests narrow_type() on fields we validate and extract structural data."""

    def _test_on_field(self, field: MetadataField) -> Callable:

        def expect(value: str, expected_value: Any, reason: str):
            output = field.narrow_type(value)
            self.assertEqual(
                output,
                expected_value,
                f'Field "{field.get_name()}" should {reason}. Input value'
                f' was: "{value}", but got coerced into {repr(output)}',
            )

        return expect

    def test_name(self):
        expect = self._test_on_field(fields.NAME)
        expect("package name", "package name", "return as-is")
        expect("", "", "not coerce empty string to `None`")

    def test_short_name(self):
        expect = self._test_on_field(fields.SHORT_NAME)
        expect("pkg-name", "pkg-name", "return as-is")
        expect("", "", "not coerce empty string to `None`")

    def test_url(self):
        expect = self._test_on_field(fields.URL)
        expect("", None, "treat empty string as None")
        expect("https://example.com/", ["https://example.com/"],
               "return valid url")
        expect(
            "https://example.com/,\nhttps://example2.com/",
            ["https://example.com/", "https://example2.com/"],
            "return multiple valid urls",
        )
        expect("file://test", [], "reject unsupported scheme")
        expect(
            "file://test,\nhttps://example.com",
            ["https://example.com"],
            "reject unsupported scheme",
        )
        expect("HTTPS://example.com", ["https://example.com"],
               "canonicalize url")
        expect("http", [], "reject invalid url")
        expect(
            "This is the canonical repo.",
            None,
            "understand the this repo is canonical message",
        )

    def test_version(self):
        expect = self._test_on_field(fields.VERSION)
        expect("", None, "treat empty string as None")
        expect("0", None, "treat invalid value as None")
        expect("varies", None, "treat invalid value as None")
        expect("see deps", None, "treat invalid value as None")
        expect("N/A", None, "N/A is treated as None")
        expect("Not applicable.", None, "N/A is treated as None")

    def test_date(self):
        expect = self._test_on_field(fields.DATE)
        expect("", None, "treat empty string as None")
        expect("0", None, "treat invalid value as None")
        expect("varies", None, "treat invalid value as None")
        expect("2024-01-02", "2024-01-02", "accepts ISO 8601 date")
        expect("2024-01-02T03:04:05Z", "2024-01-02",
               "accepts ISO 8601 date time")
        expect("Jan 2 2024", "2024-01-02", "accepts locale format")
        expect(
            "02/03/2000",
            "2000-03-02",
            "accepts ambiguous MM/DD format (better than no date info at all)",
        )
        expect("11/30/2000", "2000-11-30", "accepts unambiguous MM/DD format")

    def test_revision(self):
        expect = self._test_on_field(fields.REVISION)
        expect("", None, "treat empty string as None")
        expect("0", None, "treat invalid value as None")
        expect("varies", None, "treat invalid value as None")
        expect("see deps", None, "treat invalid value as None")
        expect("N/A", None, "N/A is treated as None")
        expect("Not applicable.", None, "N/A is treated as None")
        expect("invalid", None, "treat invalid hex as None")
        expect("123456", None, "treat too short hex as None")
        expect(
            "0123456789abcdef0123456789abcdef01234567abcabc",
            None,
            "treat long hex (>40) as None",
        )
        expect("varies", None, "treat varies as None")
        expect("see deps", None, "treat see deps as None")
        expect("N/A", None, "treat N/A as None")
        expect("Not applicable.", None, "treat 'Not applicable.' as None")
        expect("Head", None, "treat 'Head' as None")
        expect("abcdef1", "abcdef1", "leave valid hex unchanged")
        expect(
            "abcdef1abcdef1",
            "abcdef1abcdef1",
            "leave valid hex unchanged if between 7-40 chars",
        )

    def test_license(self):
        expect = self._test_on_field(fields.LICENSE)
        expect("", None, "treat empty string as None")
        expect("LICENSE-1", ["LICENSE-1"], "return as a list")
        expect("LGPL v2, BSD", ["LGPL v2", "BSD"], "return as a list")

    def test_license_file(self):
        # TODO(b/321154076): Consider excluding files that doesn't exist on
        # disk if it's not too hard.
        #
        # Right now, we return the unparsed license file field as-is.
        expect = self._test_on_field(fields.LICENSE_FILE)
        expect("src/file", "src/file", "return value as-is")

    def test_security_critical(self):
        expect = self._test_on_field(fields.SECURITY_CRITICAL)
        expect("yes", True, "understand truthy value")
        expect("Yes", True, "understand truthy value")
        expect("no", False, "understand falsey value")
        expect("No, because", False,
               "understand falsey value, with description")

    def test_shipped(self):
        expect = self._test_on_field(fields.SHIPPED)
        expect("yes", True, "understand truthy value")
        expect("Yes, but", True, "understand truthy value with extra comment")
        expect("no", False, "understand falsey value")
        expect("no, because", False,
               "understand falsey value, with extra comment")

    def test_shipped_in_chromium(self):
        expect = self._test_on_field(fields.SHIPPED_IN_CHROMIUM)
        expect("yes", True, "understand truthy value")
        expect("Yes", True, "understand truthy value")
        expect("no", False, "understand falsey value")
        expect("no, because", False,
               "understand falsey value, with extra comment")

    def test_license_android_compatible(self):
        expect = self._test_on_field(fields.LICENSE_ANDROID_COMPATIBLE)
        expect("yes", True, "understand truthy value")
        expect("Yes", True, "understand truthy value")
        expect("no", False, "understand falsey value")
        expect("no, because", False,
               "understand falsey value, with extra comment")

    def test_cpe_prefix(self):
        expect = self._test_on_field(fields.CPE_PREFIX)
        expect("unknown", None, "treat unknown as None")
        expect("Unknown", None, "treat unknown as None")
        expect("bad_cpe_format", None, "rejects invalid value")
        expect("cpe:/a:d3", "cpe:/a:d3", "accept a valid cpe prefix")
        expect("cpe:/a:D3", "cpe:/a:d3", "normalize to lowercase")

    def test_description(self):
        expect = self._test_on_field(fields.DESCRIPTION)
        expect("desc", "desc", "return value as-is")

    def test_local_modification(self):
        expect = self._test_on_field(fields.LOCAL_MODIFICATIONS)
        expect("none", False, "understands none")
        expect("(none)", False, "understands none")
        expect("not applicable", False, "understands N/A")
        expect("", False, "treat empty string as False")
        expect(
            "modified X file",
            "modified X file",
            "return value as-is if it doesn't mean no modification",
        )

    def test_dependency_data_return_as_property(self):
        dm = DependencyMetadata()
        dm.add_entry("name", "package")
        dm.add_entry("url", "git://git@example.com,\nbad_url://example.com")
        dm.add_entry("security critical", "no")
        dm.add_entry("date", "2024-01-02")
        dm.add_entry("revision", "")

        self.assertEqual(dm.name, "package")
        self.assertEqual(dm.url, ["git://git@example.com"])
        self.assertEqual(dm.security_critical, False)
        self.assertEqual(dm.date, "2024-01-02")
        self.assertEqual(dm.revision, None)
        self.assertEqual(dm.version, None)

    def test_dependency_data_repo_is_canonical(self):
        dm = DependencyMetadata()
        dm.add_entry("name", "package")
        dm.add_entry("url", "This is the canonical repo.")

        self.assertEqual(dm.url, None)
        self.assertEqual(dm.is_canonical, True)


if __name__ == "__main__":
    unittest.main()
