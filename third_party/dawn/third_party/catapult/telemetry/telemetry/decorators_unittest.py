# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from unittest import mock

from telemetry.core import platform
from telemetry import decorators
from telemetry.internal.browser import possible_browser


class FakeTest():

  def SetEnabledStrings(self, enabled_strings):
    enabled_attr_name = decorators.EnabledAttributeName(self)
    setattr(self, enabled_attr_name, enabled_strings)

  def SetDisabledStrings(self, disabled_strings):
    # pylint: disable=attribute-defined-outside-init
    disabled_attr_name = decorators.DisabledAttributeName(self)
    setattr(self, disabled_attr_name, disabled_strings)


class TestDisableDecorators(unittest.TestCase):

  def testCannotDisableClasses(self):

    class Ford():
      pass

    with self.assertRaises(TypeError):
      decorators.Disabled('example')(Ford)

  def testDisabledStringOnMethod(self):

    class Ford():

      @decorators.Disabled('windshield')
      def Drive(self):
        pass

    self.assertEqual({'windshield'},
                     decorators.GetDisabledAttributes(Ford().Drive))

    class Honda():

      @decorators.Disabled('windows', 'Drive')
      @decorators.Disabled('wheel')
      @decorators.Disabled('windows')
      def Drive(self):
        pass

    self.assertEqual({'wheel', 'Drive', 'windows'},
                     decorators.GetDisabledAttributes(Honda().Drive))
    self.assertEqual({'windshield'},
                     decorators.GetDisabledAttributes(Ford().Drive))

    class Accord(Honda):

      def Drive(self):
        pass

    class Explorer(Ford):
      pass

    self.assertEqual({'wheel', 'Drive', 'windows'},
                     decorators.GetDisabledAttributes(Honda().Drive))
    self.assertEqual({'windshield'},
                     decorators.GetDisabledAttributes(Ford().Drive))
    self.assertEqual({'windshield'},
                     decorators.GetDisabledAttributes(Explorer().Drive))
    self.assertFalse(decorators.GetDisabledAttributes(Accord().Drive))


class TestEnableDecorators(unittest.TestCase):

  def testCannotEnableClasses(self):

    class Ford():
      pass

    with self.assertRaises(TypeError):
      decorators.Disabled('example')(Ford)

  def testEnabledStringOnMethod(self):

    class Ford():

      @decorators.Enabled('windshield')
      def Drive(self):
        pass

    self.assertEqual({'windshield'},
                     decorators.GetEnabledAttributes(Ford().Drive))

    class Honda():

      @decorators.Enabled('windows', 'Drive')
      @decorators.Enabled('wheel', 'Drive')
      @decorators.Enabled('windows')
      def Drive(self):
        pass

    self.assertEqual({'wheel', 'Drive', 'windows'},
                     decorators.GetEnabledAttributes(Honda().Drive))

    class Accord(Honda):

      def Drive(self):
        pass

    class Explorer(Ford):
      pass

    self.assertEqual({'wheel', 'Drive', 'windows'},
                     decorators.GetEnabledAttributes(Honda().Drive))
    self.assertEqual({'windshield'},
                     decorators.GetEnabledAttributes(Ford().Drive))
    self.assertEqual({'windshield'},
                     decorators.GetEnabledAttributes(Explorer().Drive))
    self.assertFalse(decorators.GetEnabledAttributes(Accord().Drive))


class TestInfoDecorators(unittest.TestCase):

  def testInfoStringOnClass(self):

    @decorators.Info(emails=['owner@chromium.org'],
                     documentation_url='http://foo.com')
    class Ford():
      pass

    self.assertEqual(['owner@chromium.org'], decorators.GetEmails(Ford))

    @decorators.Info(emails=['owner2@chromium.org'])
    @decorators.Info(component='component',
                     documentation_url='http://bar.com',
                     info_blurb='Has CVT Transmission')
    class Honda():
      pass


    self.assertEqual(['owner2@chromium.org'], decorators.GetEmails(Honda))
    self.assertEqual('http://bar.com', decorators.GetDocumentationLink(Honda))
    self.assertEqual('component', decorators.GetComponent(Honda))
    self.assertEqual(['owner@chromium.org'], decorators.GetEmails(Ford))
    self.assertEqual('http://foo.com', decorators.GetDocumentationLink(Ford))
    self.assertEqual('Has CVT Transmission', decorators.GetInfoBlurb(Honda))


  def testInfoStringOnSubClass(self):

    @decorators.Info(emails=['owner@chromium.org'], component='comp',
                     documentation_url='https://car.com')
    class Car():
      pass

    class Ford(Car):
      pass

    self.assertEqual(['owner@chromium.org'], decorators.GetEmails(Car))
    self.assertEqual('comp', decorators.GetComponent(Car))
    self.assertEqual('https://car.com', decorators.GetDocumentationLink(Car))
    self.assertFalse(decorators.GetEmails(Ford))
    self.assertFalse(decorators.GetComponent(Ford))
    self.assertFalse(decorators.GetDocumentationLink(Ford))


  def testInfoWithDuplicateAttributeSetting(self):
    # The class Car below is unused. It just throws an error.
    #pylint: disable=unused-variable
    with self.assertRaises(AssertionError):
      @decorators.Info(emails=['owner2@chromium.org'])
      @decorators.Info(emails=['owner@chromium.org'], component='comp')
      class Car():
        pass


class TestShouldSkip(unittest.TestCase):

  def setUp(self):
    fake_platform = mock.Mock(spec_set=platform.Platform)
    fake_platform.GetOSName.return_value = 'os_name'
    fake_platform.GetOSVersionName.return_value = 'os_version_name'

    self.possible_browser = mock.Mock(spec_set=possible_browser.PossibleBrowser)
    self.possible_browser.browser_type = 'browser_type'
    self.possible_browser.platform = fake_platform
    self.possible_browser.supports_tab_control = False
    self.possible_browser.GetTypExpectationsTags.return_value = []

  def testEnabledStrings(self):
    test = FakeTest()

    # When no enabled_strings is given, everything should be enabled.
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['os_name'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['another_os_name'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['os_version_name'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['os_name', 'another_os_name'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['another_os_name', 'os_name'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['another_os_name', 'another_os_version_name'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['os_name-reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['another_os_name-reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['os_version_name-reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['os_name-reference', 'another_os_name-reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['another_os_name-reference', 'os_name-reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['another_os_name-reference',
                            'another_os_version_name-reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

  def testDisabledStrings(self):
    test = FakeTest()

    # When no disabled_strings is given, nothing should be disabled.
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['os_name'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['another_os_name'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['os_version_name'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['os_name', 'another_os_name'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['another_os_name', 'os_name'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['another_os_name', 'another_os_version_name'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['os_name-reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['another_os_name-reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['os_version_name-reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['os_name-reference', 'another_os_name-reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['another_os_name-reference', 'os_name-reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['another_os_name-reference',
                             'another_os_version_name-reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    self.possible_browser.GetTypExpectationsTags.return_value = ['typ_value']
    test.SetDisabledStrings(['typ_value'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

  def testReferenceEnabledStrings(self):
    self.possible_browser.browser_type = 'reference'
    test = FakeTest()

    # When no enabled_strings is given, everything should be enabled.
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['os_name-reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['another_os_name-reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['os_version_name-reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['os_name-reference', 'another_os_name-reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['another_os_name-reference', 'os_name-reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['another_os_name-reference',
                            'another_os_version_name-reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetEnabledStrings(['typ_value'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])
    self.possible_browser.GetTypExpectationsTags.return_value = ['typ_value']
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

  def testReferenceDisabledStrings(self):
    self.possible_browser.browser_type = 'reference'
    test = FakeTest()

    # When no disabled_strings is given, nothing should be disabled.
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['os_name-reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['another_os_name-reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['os_version_name-reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['os_name-reference', 'another_os_name-reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['another_os_name-reference', 'os_name-reference'])
    self.assertTrue(decorators.ShouldSkip(test, self.possible_browser)[0])

    test.SetDisabledStrings(['another_os_name-reference',
                             'another_os_version_name-reference'])
    self.assertFalse(decorators.ShouldSkip(test, self.possible_browser)[0])


class TestDeprecation(unittest.TestCase):

  @mock.patch('warnings.warn')
  def testFunctionDeprecation(self, warn_mock):

    @decorators.Deprecated(2015, 12, 1)
    def Foo(x):
      return x

    Foo(1)
    warn_mock.assert_called_with(
        'Function Foo is deprecated. It will no longer be supported on '
        'December 01, 2015. Please remove it or switch to an alternative '
        'before that time. \n',
        stacklevel=4)

  @mock.patch('warnings.warn')
  def testMethodDeprecated(self, warn_mock):

    class Bar():

      @decorators.Deprecated(2015, 12, 1, 'Testing only.')
      def Foo(self, x):
        return x

    Bar().Foo(1)
    warn_mock.assert_called_with(
        'Function Foo is deprecated. It will no longer be supported on '
        'December 01, 2015. Please remove it or switch to an alternative '
        'before that time. Testing only.\n',
        stacklevel=4)

  @mock.patch('warnings.warn')
  def testClassWithoutInitDefinedDeprecated(self, warn_mock):

    @decorators.Deprecated(2015, 12, 1)
    class Bar():

      def Foo(self, x):
        return x

    Bar().Foo(1)
    warn_mock.assert_called_with(
        'Class Bar is deprecated. It will no longer be supported on '
        'December 01, 2015. Please remove it or switch to an alternative '
        'before that time. \n',
        stacklevel=4)

  @mock.patch('warnings.warn')
  def testClassWithInitDefinedDeprecated(self, warn_mock):

    @decorators.Deprecated(2015, 12, 1)
    class Bar():

      def __init__(self):
        pass

      def Foo(self, x):
        return x

    Bar().Foo(1)
    warn_mock.assert_called_with(
        'Class Bar is deprecated. It will no longer be supported on '
        'December 01, 2015. Please remove it or switch to an alternative '
        'before that time. \n',
        stacklevel=4)

  @mock.patch('warnings.warn')
  def testInheritedClassDeprecated(self, warn_mock):

    class Ba():
      pass

    @decorators.Deprecated(2015, 12, 1)
    class Bar(Ba):

      def Foo(self, x):
        return x

    class Baz(Bar):
      pass

    Baz().Foo(1)
    warn_mock.assert_called_with(
        'Class Bar is deprecated. It will no longer be supported on '
        'December 01, 2015. Please remove it or switch to an alternative '
        'before that time. \n',
        stacklevel=4)

  def testReturnValue(self):

    class Bar():

      @decorators.Deprecated(2015, 12, 1, 'Testing only.')
      def Foo(self, x):
        return x

    self.assertEqual(5, Bar().Foo(5))
