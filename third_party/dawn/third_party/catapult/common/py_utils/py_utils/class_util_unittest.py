# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest

from py_utils import class_util


class ClassUtilTest(unittest.TestCase):

  def testClassOverridden(self):
    class Parent():
      def MethodShouldBeOverridden(self):
        pass

    class Child(Parent):
      def MethodShouldBeOverridden(self):
        pass

    self.assertTrue(class_util.IsMethodOverridden(
        Parent, Child, 'MethodShouldBeOverridden'))

  def testGrandchildOverridden(self):
    class Parent():
      def MethodShouldBeOverridden(self):
        pass

    class Child(Parent):
      pass

    class Grandchild(Child):
      def MethodShouldBeOverridden(self):
        pass

    self.assertTrue(class_util.IsMethodOverridden(
        Parent, Grandchild, 'MethodShouldBeOverridden'))

  def testClassNotOverridden(self):
    class Parent():
      def MethodShouldBeOverridden(self):
        pass

    class Child(Parent):
      def SomeOtherMethod(self):
        pass

    self.assertFalse(class_util.IsMethodOverridden(
        Parent, Child, 'MethodShouldBeOverridden'))

  def testGrandchildNotOverridden(self):
    class Parent():
      def MethodShouldBeOverridden(self):
        pass

    class Child(Parent):
      def MethodShouldBeOverridden(self):
        pass

    class Grandchild(Child):
      def SomeOtherMethod(self):
        pass

    self.assertTrue(class_util.IsMethodOverridden(
        Parent, Grandchild, 'MethodShouldBeOverridden'))

  def testClassNotPresentInParent(self):
    class Parent():
      def MethodShouldBeOverridden(self):
        pass

    class Child(Parent):
      def MethodShouldBeOverridden(self):
        pass

    self.assertRaises(
        AssertionError, class_util.IsMethodOverridden,
        Parent, Child, 'WrongMethod')

  def testInvalidClass(self):
    class Foo():
      def Bar(self):
        pass

    self.assertRaises(
        AssertionError, class_util.IsMethodOverridden, 'invalid', Foo, 'Bar')

    self.assertRaises(
        AssertionError, class_util.IsMethodOverridden, Foo, 'invalid', 'Bar')

  def testMultipleInheritance(self):
    class Aaa():
      def One(self):
        pass

    class Bbb():
      def Two(self):
        pass

    class Ccc(Aaa, Bbb):
      pass

    class Ddd():
      def Three(self):
        pass

    class Eee(Ddd):
      def Three(self):
        pass

    class Fff(Ccc, Eee):
      def One(self):
        pass

    class Ggg():
      def Four(self):
        pass

    class Hhh(Fff, Ggg):
      def Two(self):
        pass

    class Iii(Hhh):
      pass

    class Jjj(Iii):
      pass

    self.assertFalse(class_util.IsMethodOverridden(Aaa, Ccc, 'One'))
    self.assertTrue(class_util.IsMethodOverridden(Aaa, Fff, 'One'))
    self.assertTrue(class_util.IsMethodOverridden(Aaa, Hhh, 'One'))
    self.assertTrue(class_util.IsMethodOverridden(Aaa, Jjj, 'One'))
    self.assertFalse(class_util.IsMethodOverridden(Bbb, Ccc, 'Two'))
    self.assertTrue(class_util.IsMethodOverridden(Bbb, Hhh, 'Two'))
    self.assertTrue(class_util.IsMethodOverridden(Bbb, Jjj, 'Two'))
    self.assertFalse(class_util.IsMethodOverridden(Eee, Fff, 'Three'))
