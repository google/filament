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
from rsa.transform import int2bytes, bytes2int, _int2bytes


class Test_int2bytes(unittest.TestCase):
    def test_accuracy(self):
        self.assertEqual(int2bytes(123456789), b'\x07[\xcd\x15')
        self.assertEqual(_int2bytes(123456789), b'\x07[\xcd\x15')

    def test_codec_identity(self):
        self.assertEqual(bytes2int(int2bytes(123456789, 128)), 123456789)
        self.assertEqual(bytes2int(_int2bytes(123456789, 128)), 123456789)

    def test_chunk_size(self):
        self.assertEqual(int2bytes(123456789, 6), b'\x00\x00\x07[\xcd\x15')
        self.assertEqual(int2bytes(123456789, 7),
                         b'\x00\x00\x00\x07[\xcd\x15')

        self.assertEqual(_int2bytes(123456789, 6),
                         b'\x00\x00\x07[\xcd\x15')
        self.assertEqual(_int2bytes(123456789, 7),
                         b'\x00\x00\x00\x07[\xcd\x15')

    def test_zero(self):
        self.assertEqual(int2bytes(0, 4), b'\x00' * 4)
        self.assertEqual(int2bytes(0, 7), b'\x00' * 7)
        self.assertEqual(int2bytes(0), b'\x00')

        self.assertEqual(_int2bytes(0, 4), b'\x00' * 4)
        self.assertEqual(_int2bytes(0, 7), b'\x00' * 7)
        self.assertEqual(_int2bytes(0), b'\x00')

    def test_correctness_against_base_implementation(self):
        # Slow test.
        values = [
            1 << 512,
            1 << 8192,
            1 << 77,
        ]
        for value in values:
            self.assertEqual(int2bytes(value), _int2bytes(value),
                             "Boom %d" % value)
            self.assertEqual(bytes2int(int2bytes(value)),
                             value,
                             "Boom %d" % value)
            self.assertEqual(bytes2int(_int2bytes(value)),
                             value,
                             "Boom %d" % value)

    def test_raises_OverflowError_when_chunk_size_is_insufficient(self):
        self.assertRaises(OverflowError, int2bytes, 123456789, 3)
        self.assertRaises(OverflowError, int2bytes, 299999999999, 4)

        self.assertRaises(OverflowError, _int2bytes, 123456789, 3)
        self.assertRaises(OverflowError, _int2bytes, 299999999999, 4)

    def test_raises_ValueError_when_negative_integer(self):
        self.assertRaises(ValueError, int2bytes, -1)
        self.assertRaises(ValueError, _int2bytes, -1)

    def test_raises_TypeError_when_not_integer(self):
        self.assertRaises(TypeError, int2bytes, None)
        self.assertRaises(TypeError, _int2bytes, None)
