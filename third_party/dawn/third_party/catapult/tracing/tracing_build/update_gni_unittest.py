# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from tracing_build.update_gni import GniFile


class UpdateGniTests(unittest.TestCase):

  def setUp(self):
    self.file_groups = ['group1', 'group2']

  def testGniTokenizer(self):
    content = ("useless data\ngroup1 = [\n    <file list goes here>\n"
               "    ]\nNote the four spaces before the ] above")
    gni_files = GniFile(content, self.file_groups)
    self.assertEqual(3, len(gni_files._tokens))
    self.assertEqual('plain', gni_files._tokens[0].token_id)
    self.assertEqual(
        "useless data\ngroup1 = [\n", gni_files._tokens[0].data)
    self.assertEqual('group1', gni_files._tokens[1].token_id)
    self.assertEqual("    <file list goes here>\n", gni_files._tokens[1].data)
    self.assertEqual('plain', gni_files._tokens[2].token_id)
    self.assertEqual(
        "    ]\nNote the four spaces before the ] above",
        gni_files._tokens[2].data)

  def testGniFileListBuilder(self):
    gni_file = GniFile('', self.file_groups)
    existing_list = ('    "/four/spaces/indent",\n"'
                     '    "/five/spaces/but/only/first/line/matters",\n')
    new_list = ['item1', 'item2', 'item3']
    self.assertEqual(
        '    "item1",\n    "item2",\n    "item3",\n',
        gni_file._GetReplacementListAsString(existing_list, new_list))
