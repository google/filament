# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import unittest

from dashboard.services import cas_service


class RBECASServiceTest(unittest.TestCase):

  def testNormalizeDigest(self):
    digest = cas_service.RBECASService._NormalizeDigest({
        'hash': '026b992daa96202ceec74fe93e0',
        'sizeBytes': '91'
    })
    self.assertEqual(digest, {
        'hash': '026b992daa96202ceec74fe93e0',
        'sizeBytes': '91'
    })

  def testNormalizeDigest_FromProto(self):
    digest = cas_service.RBECASService._NormalizeDigest({
        'hash': '026b992daa96202ceec74fe93e0',
        'size_bytes': 91
    })
    self.assertEqual(digest, {
        'hash': '026b992daa96202ceec74fe93e0',
        'sizeBytes': '91'
    })

  def testNormalizeDigest_Empty(self):
    digest = cas_service.RBECASService._NormalizeDigest({
        'hash': '026b992daa96202ceec74fe93e0',
    })
    self.assertEqual(digest, {
        'hash': '026b992daa96202ceec74fe93e0',
        'sizeBytes': '0'
    })
