#!/usr/bin/env python
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
from rsa._compat import byte
from rsa.common import byte_size, bit_size, inverse


class TestByte(unittest.TestCase):
    def test_values(self):
        self.assertEqual(byte(0), b'\x00')
        self.assertEqual(byte(255), b'\xff')

    def test_struct_error_when_out_of_bounds(self):
        self.assertRaises(struct.error, byte, 256)
        self.assertRaises(struct.error, byte, -1)


class TestByteSize(unittest.TestCase):
    def test_values(self):
        self.assertEqual(byte_size(1 << 1023), 128)
        self.assertEqual(byte_size((1 << 1024) - 1), 128)
        self.assertEqual(byte_size(1 << 1024), 129)
        self.assertEqual(byte_size(255), 1)
        self.assertEqual(byte_size(256), 2)
        self.assertEqual(byte_size(0xffff), 2)
        self.assertEqual(byte_size(0xffffff), 3)
        self.assertEqual(byte_size(0xffffffff), 4)
        self.assertEqual(byte_size(0xffffffffff), 5)
        self.assertEqual(byte_size(0xffffffffffff), 6)
        self.assertEqual(byte_size(0xffffffffffffff), 7)
        self.assertEqual(byte_size(0xffffffffffffffff), 8)

    def test_zero(self):
        self.assertEqual(byte_size(0), 1)

    def test_bad_type(self):
        self.assertRaises(TypeError, byte_size, [])
        self.assertRaises(TypeError, byte_size, ())
        self.assertRaises(TypeError, byte_size, dict())
        self.assertRaises(TypeError, byte_size, "")
        self.assertRaises(TypeError, byte_size, None)


class TestBitSize(unittest.TestCase):
    def test_zero(self):
        self.assertEqual(bit_size(0), 0)

    def test_values(self):
        self.assertEqual(bit_size(1023), 10)
        self.assertEqual(bit_size(1024), 11)
        self.assertEqual(bit_size(1025), 11)
        self.assertEqual(bit_size(1 << 1024), 1025)
        self.assertEqual(bit_size((1 << 1024) + 1), 1025)
        self.assertEqual(bit_size((1 << 1024) - 1), 1024)

    def test_negative_values(self):
        self.assertEqual(bit_size(-1023), 10)
        self.assertEqual(bit_size(-1024), 11)
        self.assertEqual(bit_size(-1025), 11)
        self.assertEqual(bit_size(-1 << 1024), 1025)
        self.assertEqual(bit_size(-((1 << 1024) + 1)), 1025)
        self.assertEqual(bit_size(-((1 << 1024) - 1)), 1024)

    def test_bad_type(self):
        self.assertRaises(TypeError, bit_size, [])
        self.assertRaises(TypeError, bit_size, ())
        self.assertRaises(TypeError, bit_size, dict())
        self.assertRaises(TypeError, bit_size, "")
        self.assertRaises(TypeError, bit_size, None)
        self.assertRaises(TypeError, bit_size, 0.0)


class TestInverse(unittest.TestCase):
    def test_normal(self):
        self.assertEqual(3, inverse(7, 4))
        self.assertEqual(9, inverse(5, 11))

    def test_not_relprime(self):
        self.assertRaises(ValueError, inverse, 4, 8)
        self.assertRaises(ValueError, inverse, 25, 5)
