# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Code for parsing HTML.

The purpose of this module is to ensure consistency of HTML parsing
in catapult_build.
"""

import bs4


def BeautifulSoup(contents):
  # html5lib is a lenient parser; compared with the default parser,
  # it is more similar to how a web browser parses. See:
  # http://www.crummy.com/software/BeautifulSoup/bs4/doc/#installing-a-parser
  return bs4.BeautifulSoup(markup=contents, features='html5lib')
