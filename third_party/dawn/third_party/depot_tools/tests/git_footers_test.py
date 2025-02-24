#!/usr/bin/env vpython3
"""Tests for git_footers."""

from io import StringIO
import json
import os
import sys
import unittest
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import gclient_utils
import git_footers


class GitFootersTest(unittest.TestCase):
    _message = """
This is my commit message. There are many like it, but this one is mine.

My commit message is my best friend. It is my life.

"""

    _position = 'refs/heads/main@{#292272}'

    _position_footer = 'Cr-Commit-Position: %s\n' % _position

    def testFootersBasic(self):
        self.assertEqual(git_footers.split_footers('Not-A: footer'),
                         (['Not-A: footer'], [], []))
        self.assertEqual(
            git_footers.split_footers('Header\n\nActual: footer'),
            (['Header', ''], ['Actual: footer'], [('Actual', 'footer')]))
        self.assertEqual(git_footers.split_footers('\nActual: footer'),
                         ([''], ['Actual: footer'], [('Actual', 'footer')]))
        self.assertEqual(
            git_footers.split_footers('H\n\nBug:\nAlso: footer'),
            (['H', ''], ['Bug:', 'Also: footer'], [('Bug', ''),
                                                   ('Also', 'footer')]))
        self.assertEqual(git_footers.split_footers('H\n\nBug:      '),
                         (['H', ''], ['Bug:'], [('Bug', '')]))
        self.assertEqual(git_footers.split_footers('H\n\nBug: 1234     '),
                         (['H', ''], ['Bug: 1234'], [('Bug', '1234')]))
        self.assertEqual(
            git_footers.split_footers('H\n\nBug: 1234\nChange-Id: Ib4321  '),
            (['H', ''], ['Bug: 1234', 'Change-Id: Ib4321'
                         ], [('Bug', '1234'), ('Change-Id', 'Ib4321')]))

        self.assertEqual(git_footers.parse_footers(self._message), {})
        self.assertEqual(
            git_footers.parse_footers(self._message + self._position_footer),
            {'Cr-Commit-Position': [self._position]})
        self.assertEqual(
            git_footers.parse_footers(self._message + self._position_footer +
                                      self._position_footer),
            {'Cr-Commit-Position': [self._position, self._position]})
        self.assertEqual(
            git_footers.parse_footers(self._message + 'Bug:\n' +
                                      self._position_footer),
            {
                'Bug': [''],
                'Cr-Commit-Position': [self._position]
            })

    def testSkippingBadFooterLines(self):
        message = ('Title.\n'
                   '\n'
                   'Last: paragraph starts\n'
                   'It-may: contain\n'
                   'bad lines, which should be skipped\n'
                   'For: example\n'
                   '(cherry picked from)\n'
                   'And-only-valid: footers taken')
        self.assertEqual(git_footers.split_footers(message), (['Title.', ''], [
            'Last: paragraph starts', 'It-may: contain',
            'bad lines, which should be skipped', 'For: example',
            '(cherry picked from)', 'And-only-valid: footers taken'
        ], [('Last', 'paragraph starts'), ('It-may', 'contain'),
            ('For', 'example'), ('And-only-valid', 'footers taken')]))
        self.assertEqual(
            git_footers.parse_footers(message), {
                'Last': ['paragraph starts'],
                'It-May': ['contain'],
                'For': ['example'],
                'And-Only-Valid': ['footers taken']
            })

    def testAvoidingURLs(self):
        message = ('Someone accidentally put a URL in the footers.\n'
                   '\n'
                   'Followed: by\n'
                   'http://domain.tld\n'
                   'Some: footers')
        self.assertEqual(
            git_footers.split_footers(message),
            (['Someone accidentally put a URL in the footers.', ''], [
                'Followed: by', 'http://domain.tld', 'Some: footers'
            ], [('Followed', 'by'), ('Some', 'footers')]))
        self.assertEqual(git_footers.parse_footers(message), {
            'Followed': ['by'],
            'Some': ['footers']
        })

    def testSplittingLastParagraph(self):
        message = ('Title.\n'
                   '\n'
                   'The final paragraph has some normal text first.\n'
                   'Followed: by\n'
                   'nonsense trailers and\n'
                   'Some: footers')
        self.assertEqual(git_footers.split_footers(message), ([
            'Title.', '', 'The final paragraph has some normal text first.', ''
        ], ['Followed: by', 'nonsense trailers and', 'Some: footers'
            ], [('Followed', 'by'), ('Some', 'footers')]))
        self.assertEqual(git_footers.parse_footers(message), {
            'Followed': ['by'],
            'Some': ['footers']
        })

    def testGetFooterChangeId(self):
        msg = '\n'.join([
            'whatever',
            '',
            'Change-Id: ignored',
            '',  # Above is ignored because of this empty line.
            'Change-Id: Ideadbeaf'
        ])
        self.assertEqual(['Ideadbeaf'], git_footers.get_footer_change_id(msg))
        self.assertEqual([],
                         git_footers.get_footer_change_id(
                             'desc\nBUG=not-a-valid-footer\nChange-Id: Ixxx'))
        self.assertEqual(['Ixxx'],
                         git_footers.get_footer_change_id(
                             'desc\nBUG=not-a-valid-footer\n\nChange-Id: Ixxx'))

    def testAddFooterChangeId(self):
        with self.assertRaises(AssertionError):
            git_footers.add_footer_change_id('Already has\n\nChange-Id: Ixxx',
                                             'Izzz')

        self.assertEqual(
            git_footers.add_footer_change_id('header-only', 'Ixxx'),
            'header-only\n\nChange-Id: Ixxx')

        self.assertEqual(
            git_footers.add_footer_change_id('header\n\nsome: footer', 'Ixxx'),
            'header\n\nsome: footer\nChange-Id: Ixxx')

        self.assertEqual(
            git_footers.add_footer_change_id('header\n\nBUG: yy', 'Ixxx'),
            'header\n\nBUG: yy\nChange-Id: Ixxx')

        self.assertEqual(
            git_footers.add_footer_change_id('header\n\nBUG: yy\nPos: 1',
                                             'Ixxx'),
            'header\n\nBUG: yy\nChange-Id: Ixxx\nPos: 1')

        self.assertEqual(
            git_footers.add_footer_change_id('header\n\nBUG: yy\n\nPos: 1',
                                             'Ixxx'),
            'header\n\nBUG: yy\n\nPos: 1\nChange-Id: Ixxx')

        # Special case: first line is never a footer, even if it looks line one.
        self.assertEqual(
            git_footers.add_footer_change_id('header: like footer', 'Ixxx'),
            'header: like footer\n\nChange-Id: Ixxx')

        self.assertEqual(
            git_footers.add_footer_change_id('Header.\n\nBug: v8\nN=t\nT=z',
                                             'Ix'),
            'Header.\n\nBug: v8\nChange-Id: Ix\nN=t\nT=z')

    def testAddFooterChangeIdWithMultilineFooters(self):
        add_change_id = lambda lines: git_footers.add_footer_change_id(
            '\n'.join(lines), 'Ixxx')

        self.assertEqual(
            add_change_id([
                'header',
                '',
                '',
                'BUG: yy',
                'Test: hello ',
                '  world',
            ]),
            '\n'.join([
                'header',
                '',
                '',
                'BUG: yy',
                'Test: hello ',
                '  world',
                'Change-Id: Ixxx',
            ]),
        )

        self.assertEqual(
            add_change_id([
                'header',
                '',
                '',
                'BUG: yy',
                'Yeah: hello ',
                '  world',
            ]),
            '\n'.join([
                'header',
                '',
                '',
                'BUG: yy',
                'Change-Id: Ixxx',
                'Yeah: hello ',
                '  world',
            ]),
        )

        self.assertEqual(
            add_change_id([
                'header',
                '',
                '',
                'Something: ',
                '   looks great',
                'BUG: yy',
                'Test: hello ',
                '  world',
            ]),
            '\n'.join([
                'header',
                '',
                '',
                'Something: ',
                '   looks great',
                'BUG: yy',
                'Test: hello ',
                '  world',
                'Change-Id: Ixxx',
            ]),
        )

        self.assertEqual(
            add_change_id([
                'header',
                '',
                '',
                'Something: ',
                'BUG: yy',
                'Something: ',
                '   looks great',
                'Test: hello ',
                '  world',
            ]),
            '\n'.join([
                'header',
                '',
                '',
                'Something: ',
                'BUG: yy',
                'Something: ',
                '   looks great',
                'Test: hello ',
                '  world',
                'Change-Id: Ixxx',
            ]),
        )

    def testAddFooter(self):
        with self.assertRaises(ValueError):
            git_footers.add_footer('', 'Invalid Footer', 'Value')

        self.assertEqual(git_footers.add_footer('', 'Key', 'Value'),
                         '\nKey: Value')

        self.assertEqual(
            git_footers.add_footer('Header with empty line.\n\n', 'Key',
                                   'Value'),
            'Header with empty line.\n\nKey: Value')

        self.assertEqual(
            git_footers.add_footer('Top\n\nSome: footer', 'Key', 'value'),
            'Top\n\nSome: footer\nKey: value')

        self.assertEqual(
            git_footers.add_footer('Top\n\nSome: footer',
                                   'Key',
                                   'value',
                                   after_keys=['Any']),
            'Top\n\nSome: footer\nKey: value')

        self.assertEqual(
            git_footers.add_footer('Top\n\nSome: footer',
                                   'Key',
                                   'value',
                                   after_keys=['Some']),
            'Top\n\nSome: footer\nKey: value')

        self.assertEqual(
            git_footers.add_footer('Top\n\nSome: footer\nOther: footer',
                                   'Key',
                                   'value',
                                   after_keys=['Some']),
            'Top\n\nSome: footer\nKey: value\nOther: footer')

    def testRemoveFooter(self):
        self.assertEqual(git_footers.remove_footer('message', 'Key'), 'message')

        self.assertEqual(
            git_footers.remove_footer('message\n\nSome: footer', 'Key'),
            'message\n\nSome: footer')

        self.assertEqual(
            git_footers.remove_footer('message\n\nSome: footer\nKey: value',
                                      'Key'), 'message\n\nSome: footer')

        self.assertEqual(
            git_footers.remove_footer(
                'message\n\nKey: value\nSome: footer\nKey: value', 'Key'),
            'message\n\nSome: footer')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                     'not supported in non-git environment')
    @mock.patch('sys.stdout', StringIO())
    @mock.patch(
        'sys.stdin',
        StringIO('line\r\notherline\r\n\r\n\r\nFoo: baz\r\nStill: footer'))
    def testReadStdin(self):
        self.assertEqual(git_footers.main([]), 0)
        self.assertEqual(sys.stdout.getvalue(), 'Still: footer\nFoo: baz\n')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                     'not supported in non-git environment')
    @mock.patch('sys.stdin',
                StringIO('line\r\nany spaces\r\n\r\n\r\nFoo: 1\nBar: 2\nFoo: 3')
                )
    def testToJson(self):
        with gclient_utils.temporary_file() as tmp:
            self.assertEqual(git_footers.main(['--json', tmp]), 0)
            with open(tmp) as f:
                js = json.load(f)
        self.assertEqual(js, {'Foo': ['3', '1'], 'Bar': ['2']})


if __name__ == '__main__':
    unittest.main()
