# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import absolute_import
from moose.horn import horn_object

class Moose():
  def __init__(self):
    self._horn = horn_object.Horn()

  def Run(self):
    if self._horn.IsBig():
      print('I need to drop my horn! It is big!')
