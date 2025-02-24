# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


class DiagnosticRef(object):
  def __init__(self, guid):
    self._guid = guid

  @property
  def guid(self):
    return self._guid

  @property
  def has_guid(self):
    return True

  def AsDict(self):
    return self.guid

  def AsDictOrReference(self):
    return self.AsDict()
