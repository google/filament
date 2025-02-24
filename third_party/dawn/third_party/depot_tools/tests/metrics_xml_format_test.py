#!/usr/bin/env vpython3
# coding=utf-8
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
import unittest
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import gclient_paths_test
import metrics_xml_format

norm = lambda path: os.path.join(*path.split('/'))


class TestBase(gclient_paths_test.TestBase):

    def setUp(self):
        super().setUp()

        # os.path.realpath() doesn't seem to use os.path.getcwd() to compute
        # the realpath of a given path.
        #
        # This mock os.path.realpath such that it uses the mocked getcwd().
        mock.patch('os.path.realpath', self.realpath).start()
        # gclient_paths.GetPrimarysolutionPath() defaults to src.
        self.make_file_tree({'.gclient': ''})
        self.cwd = os.path.join(self.cwd, 'src')

    def realpath(self, path):
        if os.path.isabs(path):
            return path

        return os.path.join(self.getcwd(), path)


class GetMetricsDirTest(TestBase):

    def testWithAbsolutePath(self):
        top = self.getcwd()
        get = lambda path: metrics_xml_format.GetMetricsDir(
            top, os.path.join(top, norm(path)))

        self.assertTrue(get('tools/metrics/actions/abc.xml'))
        self.assertTrue(get('tools/metrics/histograms/abc.xml'))
        self.assertTrue(get('tools/metrics/structured/abc.xml'))
        self.assertTrue(get('tools/metrics/ukm/abc.xml'))

        self.assertFalse(get('tools/test/metrics/actions/abc.xml'))
        self.assertFalse(get('tools/test/metrics/histograms/abc.xml'))
        self.assertFalse(get('tools/test/metrics/structured/abc.xml'))
        self.assertFalse(get('tools/test/metrics/ukm/abc.xml'))

    def testWithRelativePaths(self):
        top = self.getcwd()
        # chdir() to tools so that relative paths from tools become valid.
        self.cwd = os.path.join(self.cwd, 'tools')
        get = lambda path: metrics_xml_format.GetMetricsDir(top, path)
        self.assertTrue(get(norm('metrics/actions/abc.xml')))
        self.assertFalse(get(norm('abc.xml')))


class FindMetricsXMLFormatTool(TestBase):

    def testWithMetricsXML(self):
        top = self.getcwd()
        findTool = metrics_xml_format.FindMetricsXMLFormatterTool

        self.assertEqual(
            findTool(norm('tools/metrics/actions/abc.xml')),
            os.path.join(top, norm('tools/metrics/actions/pretty_print.py')),
        )

        # same test, but with an absolute path.
        self.assertEqual(
            findTool(os.path.join(top, norm('tools/metrics/actions/abc.xml'))),
            os.path.join(top, norm('tools/metrics/actions/pretty_print.py')),
        )

    def testWthNonMetricsXML(self):
        findTool = metrics_xml_format.FindMetricsXMLFormatterTool
        self.assertEqual(findTool(norm('tools/metrics/test/abc.xml')), '')

    def testWithNonCheckout(self):
        findTool = metrics_xml_format.FindMetricsXMLFormatterTool
        self.cwd = self.root
        self.assertEqual(findTool(norm('tools/metrics/actions/abc.xml')), '')

    def testWithDifferentCheckout(self):
        findTool = metrics_xml_format.FindMetricsXMLFormatterTool
        checkout2 = os.path.join(self.root, '..', self._testMethodName + '2',
                                 'src')
        self.assertEqual(
            # this is the case the tool was given a file path that is located
            # in a different checkout folder.
            findTool(
                os.path.join(checkout2, norm('tools/metrics/actions/abc.xml'))),
            '',
        )

    def testSupportedHistogramsXML(self):
        top = self.getcwd()
        findTool = metrics_xml_format.FindMetricsXMLFormatterTool
        self.assertEqual(
            findTool(norm('tools/metrics/histograms/enums.xml')),
            os.path.join(top, norm('tools/metrics/histograms/pretty_print.py')),
        )
        self.assertEqual(
            findTool(norm('tools/metrics/histograms/tests/histograms.xml')),
            os.path.join(top, norm('tools/metrics/histograms/pretty_print.py')),
        )

    def testNotSupportedHistogramsXML(self):
        tool = metrics_xml_format.FindMetricsXMLFormatterTool(
            norm('tools/metrics/histograms/NO.xml'))
        self.assertEqual(tool, '')


if __name__ == '__main__':
    unittest.main()
