# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import re
from py_vulcanize import style_sheet


class HTMLGenerationController(object):

  def __init__(self):
    self.current_module = None

  def GetHTMLForStylesheetHRef(self, href):  # pylint: disable=unused-argument
    return None

  def GetHTMLForInlineStylesheet(self, contents):
    if self.current_module is None:
      if re.search(r'url\(.+\)', contents):
        raise Exception(
            'Default HTMLGenerationController cannot handle inline style urls')
      return contents

    module_dirname = os.path.dirname(self.current_module.resource.absolute_path)
    ss = style_sheet.ParsedStyleSheet(
        self.current_module.loader, module_dirname, contents)
    return ss.contents_with_inlined_images
