# -*- coding: utf-8 -*-
#
#  Copyright 2011 Sybren A. Stüvel <sybren@stuvel.eu>
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

"""Tests string operations."""

from __future__ import absolute_import

import unittest

import rsa

unicode_string = u"Euro=\u20ac ABCDEFGHIJKLMNOPQRSTUVWXYZ"


class StringTest(unittest.TestCase):
    def setUp(self):
        (self.pub, self.priv) = rsa.newkeys(384)

    def test_enc_dec(self):
        message = unicode_string.encode('utf-8')
        print("\tMessage:   %s" % message)

        encrypted = rsa.encrypt(message, self.pub)
        print("\tEncrypted: %s" % encrypted)

        decrypted = rsa.decrypt(encrypted, self.priv)
        print("\tDecrypted: %s" % decrypted)

        self.assertEqual(message, decrypted)
