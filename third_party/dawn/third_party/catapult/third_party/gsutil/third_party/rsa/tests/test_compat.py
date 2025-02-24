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

import unittest
import struct

from rsa._compat import byte, is_bytes, range, xor_bytes


class TestByte(unittest.TestCase):
    """Tests for single bytes."""

    def test_byte(self):
        for i in range(256):
            byt = byte(i)
            self.assertTrue(is_bytes(byt))
            self.assertEqual(ord(byt), i)

    def test_raises_StructError_on_overflow(self):
        self.assertRaises(struct.error, byte, 256)
        self.assertRaises(struct.error, byte, -1)

    def test_byte_literal(self):
        self.assertIsInstance(b'abc', bytes)


class TestBytes(unittest.TestCase):
    """Tests for bytes objects."""

    def setUp(self):
        self.b1 = b'\xff\xff\xff\xff'
        self.b2 = b'\x00\x00\x00\x00'
        self.b3 = b'\xf0\xf0\xf0\xf0'
        self.b4 = b'\x4d\x23\xca\xe2'
        self.b5 = b'\x9b\x61\x3b\xdc'
        self.b6 = b'\xff\xff'

        self.byte_strings = (self.b1, self.b2, self.b3, self.b4, self.b5, self.b6)

    def test_xor_bytes(self):
        self.assertEqual(xor_bytes(self.b1, self.b2), b'\xff\xff\xff\xff')
        self.assertEqual(xor_bytes(self.b1, self.b3), b'\x0f\x0f\x0f\x0f')
        self.assertEqual(xor_bytes(self.b1, self.b4), b'\xb2\xdc\x35\x1d')
        self.assertEqual(xor_bytes(self.b1, self.b5), b'\x64\x9e\xc4\x23')
        self.assertEqual(xor_bytes(self.b2, self.b3), b'\xf0\xf0\xf0\xf0')
        self.assertEqual(xor_bytes(self.b2, self.b4), b'\x4d\x23\xca\xe2')
        self.assertEqual(xor_bytes(self.b2, self.b5), b'\x9b\x61\x3b\xdc')
        self.assertEqual(xor_bytes(self.b3, self.b4), b'\xbd\xd3\x3a\x12')
        self.assertEqual(xor_bytes(self.b3, self.b5), b'\x6b\x91\xcb\x2c')
        self.assertEqual(xor_bytes(self.b4, self.b5), b'\xd6\x42\xf1\x3e')

    def test_xor_bytes_length(self):
        self.assertEqual(xor_bytes(self.b1, self.b6), b'\x00\x00')
        self.assertEqual(xor_bytes(self.b2, self.b6), b'\xff\xff')
        self.assertEqual(xor_bytes(self.b3, self.b6), b'\x0f\x0f')
        self.assertEqual(xor_bytes(self.b4, self.b6), b'\xb2\xdc')
        self.assertEqual(xor_bytes(self.b5, self.b6), b'\x64\x9e')
        self.assertEqual(xor_bytes(self.b6, b''), b'')

    def test_xor_bytes_commutative(self):
        for first in self.byte_strings:
            for second in self.byte_strings:
                min_length = min(len(first), len(second))
                result = xor_bytes(first, second)

                self.assertEqual(result, xor_bytes(second, first))
                self.assertEqual(len(result), min_length)
