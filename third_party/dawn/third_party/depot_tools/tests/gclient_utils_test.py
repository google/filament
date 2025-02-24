#!/usr/bin/env vpython3
# coding=utf-8
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import io
import os
import sys
import unittest
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import gclient_utils
import subprocess2
from testing_support import trial_dir

# TODO: Should fix these warnings.
# pylint: disable=line-too-long


class CheckCallAndFilterTestCase(unittest.TestCase):
    class ProcessIdMock(object):
        def __init__(self, test_string, return_code=0):
            self.stdout = test_string.encode('utf-8')
            self.pid = 9284
            self.return_code = return_code

        def wait(self):
            return self.return_code

    def PopenMock(self, *args, **kwargs):
        kid = self.kids.pop(0)
        stdout = kwargs.get('stdout')
        os.write(stdout, kid.stdout)
        return kid

    def setUp(self):
        super(CheckCallAndFilterTestCase, self).setUp()
        self.printfn = io.StringIO()
        self.stdout = io.BytesIO()
        self.kids = []
        mock.patch('sys.stdout', mock.Mock()).start()
        mock.patch('sys.stdout.buffer', self.stdout).start()
        mock.patch('sys.stdout.isatty', return_value=False).start()
        mock.patch('builtins.print', self.printfn.write).start()
        mock.patch('sys.stdout.flush', lambda: None).start()
        self.addCleanup(mock.patch.stopall)

    @mock.patch('subprocess2.Popen')
    def testCheckCallAndFilter(self, mockPopen):
        cwd = 'bleh'
        args = ['boo', 'foo', 'bar']
        test_string = 'ahah\naccb\nallo\naddb\n✔'

        self.kids = [self.ProcessIdMock(test_string)]
        mockPopen.side_effect = self.PopenMock

        line_list = []
        result = gclient_utils.CheckCallAndFilter(args,
                                                  cwd=cwd,
                                                  show_header=True,
                                                  always_show_header=True,
                                                  filter_fn=line_list.append)

        self.assertEqual(result, test_string.encode('utf-8'))
        self.assertEqual(line_list, [
            '________ running \'boo foo bar\' in \'bleh\'\n', 'ahah', 'accb',
            'allo', 'addb', '✔'
        ])
        self.assertEqual(self.stdout.getvalue(), b'')

        kall = mockPopen.call_args
        self.assertEqual(kall.args, (args, ))
        self.assertEqual(kall.kwargs['cwd'], cwd)
        self.assertEqual(kall.kwargs['stdout'], mock.ANY)
        self.assertEqual(kall.kwargs['stderr'], subprocess2.STDOUT)
        self.assertEqual(kall.kwargs['bufsize'], 0)
        self.assertIn('env', kall.kwargs)
        self.assertIn('COLUMNS', kall.kwargs['env'])
        self.assertIn('LINES', kall.kwargs['env'])

    @mock.patch('time.sleep')
    @mock.patch('subprocess2.Popen')
    def testCheckCallAndFilter_RetryOnce(self, mockPopen, mockTime):
        cwd = 'bleh'
        args = ['boo', 'foo', 'bar']
        test_string = 'ahah\naccb\nallo\naddb\n✔'

        self.kids = [
            self.ProcessIdMock(test_string, 1),
            self.ProcessIdMock(test_string, 0)
        ]
        mockPopen.side_effect = self.PopenMock

        line_list = []
        result = gclient_utils.CheckCallAndFilter(args,
                                                  cwd=cwd,
                                                  show_header=True,
                                                  always_show_header=True,
                                                  filter_fn=line_list.append,
                                                  retry=True)

        self.assertEqual(result, test_string.encode('utf-8'))

        self.assertEqual(line_list, [
            '________ running \'boo foo bar\' in \'bleh\'\n',
            'ahah',
            'accb',
            'allo',
            'addb',
            '✔',
            '________ running \'boo foo bar\' in \'bleh\' attempt 2 / 2\n',
            'ahah',
            'accb',
            'allo',
            'addb',
            '✔',
        ])

        mockTime.assert_called_with(gclient_utils.RETRY_INITIAL_SLEEP)

        for i in range(2):
            kall = mockPopen.mock_calls[i]
            self.assertEqual(kall.args, (args, ))
            self.assertEqual(kall.kwargs['cwd'], cwd)
            self.assertEqual(kall.kwargs['stdout'], mock.ANY)
            self.assertEqual(kall.kwargs['stderr'], subprocess2.STDOUT)
            self.assertEqual(kall.kwargs['bufsize'], 0)
            self.assertIn('env', kall.kwargs)
            self.assertIn('COLUMNS', kall.kwargs['env'])
            self.assertIn('LINES', kall.kwargs['env'])

        self.assertEqual(self.stdout.getvalue(), b'')
        self.assertEqual(
            self.printfn.getvalue(),
            'WARNING: subprocess \'"boo" "foo" "bar"\' in bleh failed; will retry '
            'after a short nap...')

    @mock.patch('subprocess2.Popen')
    def testCheckCallAndFilter_PrintStdout(self, mockPopen):
        cwd = 'bleh'
        args = ['boo', 'foo', 'bar']
        test_string = 'ahah\naccb\nallo\naddb\n✔'

        self.kids = [self.ProcessIdMock(test_string)]
        mockPopen.side_effect = self.PopenMock

        result = gclient_utils.CheckCallAndFilter(args,
                                                  cwd=cwd,
                                                  show_header=True,
                                                  always_show_header=True,
                                                  print_stdout=True)

        self.assertEqual(result, test_string.encode('utf-8'))
        self.assertEqual(self.stdout.getvalue().splitlines(), [
            b"________ running 'boo foo bar' in 'bleh'",
            b'ahah',
            b'accb',
            b'allo',
            b'addb',
            b'\xe2\x9c\x94',
        ])


class AnnotatedTestCase(unittest.TestCase):
    def setUp(self):
        self.out = gclient_utils.MakeFileAnnotated(io.BytesIO())
        self.annotated = gclient_utils.MakeFileAnnotated(io.BytesIO(),
                                                         include_zero=True)

    def testWrite(self):
        test_cases = [
            ('test string\n', b'test string\n'),
            (b'test string\n', b'test string\n'),
            ('✔\n', b'\xe2\x9c\x94\n'),
            (b'\xe2\x9c\x94\n', b'\xe2\x9c\x94\n'),
            ('first line\nsecondline\n', b'first line\nsecondline\n'),
            (b'first line\nsecondline\n', b'first line\nsecondline\n'),
        ]

        for test_input, expected_output in test_cases:
            out = gclient_utils.MakeFileAnnotated(io.BytesIO())
            out.write(test_input)
            self.assertEqual(out.getvalue(), expected_output)

    def testWrite_Annotated(self):
        test_cases = [
            ('test string\n', b'0>test string\n'),
            (b'test string\n', b'0>test string\n'),
            ('✔\n', b'0>\xe2\x9c\x94\n'),
            (b'\xe2\x9c\x94\n', b'0>\xe2\x9c\x94\n'),
            ('first line\nsecondline\n', b'0>first line\n0>secondline\n'),
            (b'first line\nsecondline\n', b'0>first line\n0>secondline\n'),
        ]

        for test_input, expected_output in test_cases:
            out = gclient_utils.MakeFileAnnotated(io.BytesIO(),
                                                  include_zero=True)
            out.write(test_input)
            self.assertEqual(out.getvalue(), expected_output)

    def testByteByByteInput(self):
        self.out.write(b'\xe2')
        self.out.write(b'\x9c')
        self.out.write(b'\x94')
        self.out.write(b'\n')
        self.out.write(b'\xe2')
        self.out.write(b'\n')
        self.assertEqual(self.out.getvalue(), b'\xe2\x9c\x94\n\xe2\n')

    def testByteByByteInput_Annotated(self):
        self.annotated.write(b'\xe2')
        self.annotated.write(b'\x9c')
        self.annotated.write(b'\x94')
        self.annotated.write(b'\n')
        self.annotated.write(b'\xe2')
        self.annotated.write(b'\n')
        self.assertEqual(self.annotated.getvalue(), b'0>\xe2\x9c\x94\n0>\xe2\n')

    def testFlush_Annotated(self):
        self.annotated.write(b'first line\nsecond line')
        self.assertEqual(self.annotated.getvalue(), b'0>first line\n')
        self.annotated.flush()
        self.assertEqual(self.annotated.getvalue(),
                         b'0>first line\n0>second line\n')


class SplitUrlRevisionTestCase(unittest.TestCase):
    def testSSHUrl(self):
        url = "ssh://test@example.com/test.git"
        rev = "ac345e52dc"
        out_url, out_rev = gclient_utils.SplitUrlRevision(url)
        self.assertEqual(out_rev, None)
        self.assertEqual(out_url, url)
        out_url, out_rev = gclient_utils.SplitUrlRevision("%s@%s" % (url, rev))
        self.assertEqual(out_rev, rev)
        self.assertEqual(out_url, url)
        url = "ssh://example.com/test.git"
        out_url, out_rev = gclient_utils.SplitUrlRevision(url)
        self.assertEqual(out_rev, None)
        self.assertEqual(out_url, url)
        out_url, out_rev = gclient_utils.SplitUrlRevision("%s@%s" % (url, rev))
        self.assertEqual(out_rev, rev)
        self.assertEqual(out_url, url)
        url = "ssh://example.com/git/test.git"
        out_url, out_rev = gclient_utils.SplitUrlRevision(url)
        self.assertEqual(out_rev, None)
        self.assertEqual(out_url, url)
        out_url, out_rev = gclient_utils.SplitUrlRevision("%s@%s" % (url, rev))
        self.assertEqual(out_rev, rev)
        self.assertEqual(out_url, url)
        rev = "test-stable"
        out_url, out_rev = gclient_utils.SplitUrlRevision("%s@%s" % (url, rev))
        self.assertEqual(out_rev, rev)
        self.assertEqual(out_url, url)
        url = "ssh://user-name@example.com/~/test.git"
        out_url, out_rev = gclient_utils.SplitUrlRevision(url)
        self.assertEqual(out_rev, None)
        self.assertEqual(out_url, url)
        out_url, out_rev = gclient_utils.SplitUrlRevision("%s@%s" % (url, rev))
        self.assertEqual(out_rev, rev)
        self.assertEqual(out_url, url)
        url = "ssh://user-name@example.com/~username/test.git"
        out_url, out_rev = gclient_utils.SplitUrlRevision(url)
        self.assertEqual(out_rev, None)
        self.assertEqual(out_url, url)
        out_url, out_rev = gclient_utils.SplitUrlRevision("%s@%s" % (url, rev))
        self.assertEqual(out_rev, rev)
        self.assertEqual(out_url, url)
        url = "git@github.com:dart-lang/spark.git"
        out_url, out_rev = gclient_utils.SplitUrlRevision(url)
        self.assertEqual(out_rev, None)
        self.assertEqual(out_url, url)
        out_url, out_rev = gclient_utils.SplitUrlRevision("%s@%s" % (url, rev))
        self.assertEqual(out_rev, rev)
        self.assertEqual(out_url, url)

    def testSVNUrl(self):
        url = "svn://example.com/test"
        rev = "ac345e52dc"
        out_url, out_rev = gclient_utils.SplitUrlRevision(url)
        self.assertEqual(out_rev, None)
        self.assertEqual(out_url, url)
        out_url, out_rev = gclient_utils.SplitUrlRevision("%s@%s" % (url, rev))
        self.assertEqual(out_rev, rev)
        self.assertEqual(out_url, url)


class ExtracRefNameTest(unittest.TestCase):
    def testMatchFound(self):
        self.assertEqual(
            'main',
            gclient_utils.ExtractRefName('origin', 'refs/remote/origin/main'))
        self.assertEqual(
            '1234', gclient_utils.ExtractRefName('origin', 'refs/tags/1234'))
        self.assertEqual(
            'chicken',
            gclient_utils.ExtractRefName('origin', 'refs/heads/chicken'))

    def testNoMatch(self):
        self.assertIsNone(gclient_utils.ExtractRefName('origin', 'abcbbb1234'))


class GClientUtilsTest(trial_dir.TestCase):
    def testHardToDelete(self):
        # Use the fact that tearDown will delete the directory to make it hard
        # to do so.
        l1 = os.path.join(self.root_dir, 'l1')
        l2 = os.path.join(l1, 'l2')
        l3 = os.path.join(l2, 'l3')
        f3 = os.path.join(l3, 'f3')
        os.mkdir(l1)
        os.mkdir(l2)
        os.mkdir(l3)
        gclient_utils.FileWrite(f3, 'foo')
        os.chmod(f3, 0)
        os.chmod(l3, 0)
        os.chmod(l2, 0)
        os.chmod(l1, 0)

    def testUpgradeToHttps(self):
        values = [
            ['', ''],
            [None, None],
            ['foo', 'https://foo'],
            ['http://foo', 'https://foo'],
            ['foo/', 'https://foo/'],
            ['ssh-svn://foo', 'ssh-svn://foo'],
            ['ssh-svn://foo/bar/', 'ssh-svn://foo/bar/'],
            ['codereview.chromium.org', 'https://codereview.chromium.org'],
            ['codereview.chromium.org/', 'https://codereview.chromium.org/'],
            [
                'chromium-review.googlesource.com',
                'https://chromium-review.googlesource.com'
            ],
            [
                'chromium-review.googlesource.com/',
                'https://chromium-review.googlesource.com/'
            ],
            ['http://foo:10000', 'http://foo:10000'],
            ['http://foo:10000/bar', 'http://foo:10000/bar'],
            ['foo:10000', 'http://foo:10000'],
            ['foo:', 'https://foo:'],
        ]
        for content, expected in values:
            self.assertEqual(expected, gclient_utils.UpgradeToHttps(content))

    def testParseCodereviewSettingsContent(self):
        values = [
            ['# bleh\n', {}],
            ['\t# foo : bar\n', {}],
            ['Foo:bar', {
                'Foo': 'bar'
            }],
            ['Foo:bar:baz\n', {
                'Foo': 'bar:baz'
            }],
            [' Foo : bar ', {
                'Foo': 'bar'
            }],
            [' Foo : bar \n', {
                'Foo': 'bar'
            }],
            ['a:b\n\rc:d\re:f', {
                'a': 'b',
                'c': 'd',
                'e': 'f'
            }],
            ['an_url:http://value/', {
                'an_url': 'http://value/'
            }],
            [
                'CODE_REVIEW_SERVER : http://r/s', {
                    'CODE_REVIEW_SERVER': 'https://r/s'
                }
            ],
            ['VIEW_VC:http://r/s', {
                'VIEW_VC': 'https://r/s'
            }],
        ]
        for content, expected in values:
            self.assertEqual(
                expected, gclient_utils.ParseCodereviewSettingsContent(content))

    def testFileRead_Bytes(self):
        with gclient_utils.temporary_file() as tmp:
            gclient_utils.FileWrite(tmp,
                                    b'foo \xe2\x9c bar',
                                    mode='wb',
                                    encoding=None)
            self.assertEqual('foo \ufffd bar', gclient_utils.FileRead(tmp))

    def testFileRead_Unicode(self):
        with gclient_utils.temporary_file() as tmp:
            gclient_utils.FileWrite(tmp, 'foo ✔ bar')
            self.assertEqual('foo ✔ bar', gclient_utils.FileRead(tmp))

    def testTemporaryFile(self):
        with gclient_utils.temporary_file() as tmp:
            gclient_utils.FileWrite(tmp, 'test')
            self.assertEqual('test', gclient_utils.FileRead(tmp))
        self.assertFalse(os.path.exists(tmp))

    def testMergeConditions(self):
        self.assertEqual(None, gclient_utils.merge_conditions(None, None))

        self.assertEqual('foo', gclient_utils.merge_conditions('foo', None))

        self.assertEqual('foo', gclient_utils.merge_conditions(None, 'foo'))

        self.assertEqual('(foo) and (bar)',
                         gclient_utils.merge_conditions('foo', 'bar'))

        self.assertEqual('(foo or bar) and (baz)',
                         gclient_utils.merge_conditions('foo or bar', 'baz'))

if __name__ == '__main__':
    unittest.main()

# vim: ts=2:sw=2:tw=80:et:
