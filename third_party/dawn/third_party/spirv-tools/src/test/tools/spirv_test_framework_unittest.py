# Copyright (c) 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Tests for the spirv test framework module."""

from spirv_test_framework import get_all_test_methods, get_all_superclasses
import unittest

# Classes to be used in testing get_all_{superclasses|test_methods}()


class Root:

    def check_root(self):
        pass


class A(Root):

    def check_a(self):
        pass


class B(Root):

    def check_b(self):
        pass


class C(Root):

    def check_c(self):
        pass


class D(Root):

    def check_d(self):
        pass


class E(Root):

    def check_e(self):
        pass


class H(B, C, D):

    def check_h(self):
        pass


class I(E):

    def check_i(self):
        pass


class O(H, I):

    def check_o(self):
        pass


class U(A, O):

    def check_u(self):
        pass


class X(U, A):

    def check_x(self):
        pass


class R1:

    def check_r1(self):
        pass


class R2:

    def check_r2(self):
        pass


class Multi(R1, R2):

    def check_multi(self):
        pass


class TestSpirvTestFramework(unittest.TestCase):
    def test_get_all_superclasses(self):
        self.assertEqual(get_all_superclasses(A), [Root])
        self.assertEqual(get_all_superclasses(B), [Root])
        self.assertEqual(get_all_superclasses(C), [Root])
        self.assertEqual(get_all_superclasses(D), [Root])
        self.assertEqual(get_all_superclasses(E), [Root])

        self.assertEqual(get_all_superclasses(H), [Root, B, C, D])
        self.assertEqual(get_all_superclasses(I), [Root, E])

        self.assertEqual(get_all_superclasses(O), [Root, B, C, D, E, H, I])

        self.assertEqual(get_all_superclasses(
            U), [Root, B, C, D, E, H, I, A, O])
        self.assertEqual(get_all_superclasses(
            X), [Root, B, C, D, E, H, I, A, O, U])

        self.assertEqual(get_all_superclasses(Multi), [R1, R2])

    def test_get_all_methods(self):
        self.assertEqual(get_all_test_methods(A), ['check_root', 'check_a'])
        self.assertEqual(get_all_test_methods(B), ['check_root', 'check_b'])
        self.assertEqual(get_all_test_methods(C), ['check_root', 'check_c'])
        self.assertEqual(get_all_test_methods(D), ['check_root', 'check_d'])
        self.assertEqual(get_all_test_methods(E), ['check_root', 'check_e'])

        self.assertEqual(
            get_all_test_methods(H),
            ['check_root', 'check_b', 'check_c', 'check_d', 'check_h'])
        self.assertEqual(get_all_test_methods(
            I), ['check_root', 'check_e', 'check_i'])

        self.assertEqual(
            get_all_test_methods(O), [
                'check_root', 'check_b', 'check_c', 'check_d', 'check_e', 'check_h',
                'check_i', 'check_o'
            ])

        self.assertEqual(
            get_all_test_methods(U), [
                'check_root', 'check_b', 'check_c', 'check_d', 'check_e', 'check_h',
                'check_i', 'check_a', 'check_o', 'check_u'
            ])

        self.assertEqual(
            get_all_test_methods(X), [
                'check_root', 'check_b', 'check_c', 'check_d', 'check_e', 'check_h',
                'check_i', 'check_a', 'check_o', 'check_u', 'check_x'
            ])

        self.assertEqual(
            get_all_test_methods(Multi), ['check_r1', 'check_r2', 'check_multi'])
