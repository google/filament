#!/usr/bin/env vpython3
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# [VPYTHON:BEGIN]
# python_version: "3.8"
# [VPYTHON:END]
"""Tests for Bazel launcher."""

import os
from pathlib import Path
import site
import sys
import unittest

DEPOT_TOOLS_DIR = Path(__file__).resolve().parent.parent
site.addsitedir(DEPOT_TOOLS_DIR)

import bazel
from testing_support import trial_dir


class FindCrosUnittest(trial_dir.TestCase):
    """Test the _find_bazel_cros function."""
    def setUp(self):
        """Create the checkout and chromite files."""
        super().setUp()
        self.checkout_dir = Path(self.root_dir) / "chromiumos"
        self.chromite_dir = self.checkout_dir / "chromite"
        self.launcher = self.chromite_dir / "bin" / "bazel"
        self.launcher.parent.mkdir(exist_ok=True, parents=True)
        self.launcher.write_bytes(b"")
        self.launcher.chmod(0o775)
        self.orig_dir = Path.cwd()

    def tearDown(self):
        os.chdir(self.orig_dir)
        super().tearDown()

    def test_at_checkout_base(self):
        """Test we find the launcher at the base of the checkout."""
        os.chdir(self.checkout_dir)
        self.assertEqual(bazel._find_bazel_cros(), self.launcher)

    def test_in_checkout_subdir(self):
        """Test we find the launcher in a subdir of the checkout."""
        os.chdir(self.chromite_dir)
        self.assertEqual(bazel._find_bazel_cros(), self.launcher)

    def test_out_of_checkout(self):
        """Test we don't find the launcher outside of the checkout."""
        os.chdir(self.root_dir)
        self.assertIsNone(bazel._find_bazel_cros())


class FindPathUnittest(trial_dir.TestCase):
    """Test the _find_next_bazel_in_path function."""
    def setUp(self):
        """Create the checkout and chromite files."""
        super().setUp()

        self.bin_dir = Path(self.root_dir) / "bin"
        self.bin_dir.mkdir(exist_ok=True, parents=True)
        self.orig_path = os.environ.get("PATH", os.defpath)

        # DEPOT_TOOLS_DIR is located twice in PATH for spice.
        os.environ["PATH"] = os.pathsep.join([
            str(DEPOT_TOOLS_DIR),
            str(self.bin_dir),
            str(DEPOT_TOOLS_DIR),
        ])

    def tearDown(self):
        """Restore actions from setUp()."""
        os.environ["PATH"] = self.orig_path

    def test_not_in_path(self):
        """Test we don't find anything in PATH when not present."""
        self.assertIsNone(bazel._find_next_bazel_in_path())

    def test_in_path(self):
        """Test we find the next Bazel in PATH when present."""
        if sys.platform == "win32":
            launcher = self.bin_dir / "bazel.exe"
        else:
            launcher = self.bin_dir / "bazel"
        launcher.write_bytes(b"")
        launcher.chmod(0o755)
        self.assertEqual(bazel._find_next_bazel_in_path(), launcher)


if __name__ == '__main__':
    unittest.main()
