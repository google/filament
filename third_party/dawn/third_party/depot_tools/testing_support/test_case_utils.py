# Copyright (c) 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Simplify unit tests based on pymox."""

import os
import random
import string


class TestCaseUtils(object):
    """Base class with some additional functionalities. People will usually want
    to use SuperMoxTestBase instead."""
    # Backup the separator in case it gets mocked
    _OS_SEP = os.sep
    _RANDOM_CHOICE = random.choice
    _RANDOM_RANDINT = random.randint
    _STRING_LETTERS = string.ascii_letters

    ## Some utilities for generating arbitrary arguments.
    def String(self, max_length):
        return ''.join([
            self._RANDOM_CHOICE(self._STRING_LETTERS)
            for _ in range(self._RANDOM_RANDINT(1, max_length))
        ])

    def Strings(self, max_arg_count, max_arg_length):
        return [self.String(max_arg_length) for _ in range(max_arg_count)]

    def Args(self, max_arg_count=8, max_arg_length=16):
        return self.Strings(max_arg_count,
                            self._RANDOM_RANDINT(1, max_arg_length))

    def _DirElts(self, max_elt_count=4, max_elt_length=8):
        return self._OS_SEP.join(self.Strings(max_elt_count, max_elt_length))

    def Dir(self, max_elt_count=4, max_elt_length=8):
        return (self._RANDOM_CHOICE(
            (self._OS_SEP, '')) + self._DirElts(max_elt_count, max_elt_length))

    def RootDir(self, max_elt_count=4, max_elt_length=8):
        return self._OS_SEP + self._DirElts(max_elt_count, max_elt_length)

    def compareMembers(self, obj, members):
        """If you add a member, be sure to add the relevant test!"""
        # Skip over members starting with '_' since they are usually not meant
        # to be for public use.
        actual_members = [x for x in sorted(dir(obj)) if not x.startswith('_')]
        expected_members = sorted(members)
        if actual_members != expected_members:
            diff = ([i for i in actual_members if i not in expected_members] +
                    [i for i in expected_members if i not in actual_members])
            print(diff, file=sys.stderr)
        # pylint: disable=no-member
        self.assertEqual(actual_members, expected_members)

    def setUp(self):
        self.root_dir = self.Dir()
        self.args = self.Args()
        self.relpath = self.String(200)
