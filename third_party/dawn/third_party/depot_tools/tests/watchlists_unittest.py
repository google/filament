#!/usr/bin/env vpython3
# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for watchlists.py."""

# pylint: disable=E1103,no-value-for-parameter,protected-access

import os
import sys
import unittest
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import watchlists


class WatchlistsTest(unittest.TestCase):
    def setUp(self):
        super(WatchlistsTest, self).setUp()
        mock.patch('watchlists.Watchlists._HasWatchlistsFile').start()
        mock.patch('watchlists.Watchlists._ContentsOfWatchlistsFile').start()
        mock.patch('watchlists.logging.error').start()
        self.addCleanup(mock.patch.stopall)

    def testMissingWatchlistsFileOK(self):
        """Test that we act gracefully if WATCHLISTS file is missing."""
        watchlists.Watchlists._HasWatchlistsFile.return_value = False

        wl = watchlists.Watchlists('/some/random/path')
        self.assertEqual(wl.GetWatchersForPaths(['some_path']), [])

    def testGarbledWatchlistsFileOK(self):
        """Test that we act gracefully if WATCHLISTS file is garbled."""
        contents = 'some garbled and unwanted text'
        watchlists.Watchlists._HasWatchlistsFile.return_value = True
        watchlists.Watchlists._ContentsOfWatchlistsFile.return_value = contents

        wl = watchlists.Watchlists('/a/path')
        self.assertEqual(wl.GetWatchersForPaths(['some_path']), [])

    def testNoWatchers(self):
        contents = \
          """{
        'WATCHLIST_DEFINITIONS': {
          'a_module': {
            'filepath': 'a_module',
          },
        },

        'WATCHLISTS': {
          'a_module': [],
        },
      } """
        watchlists.Watchlists._HasWatchlistsFile.return_value = True
        watchlists.Watchlists._ContentsOfWatchlistsFile.return_value = contents

        wl = watchlists.Watchlists('/a/path')
        self.assertEqual(wl.GetWatchersForPaths(['a_module']), [])

    def testValidWatcher(self):
        watchers = ['abc@def.com', 'x1@xyz.org']
        contents = \
          """{
        'WATCHLIST_DEFINITIONS': {
          'a_module': {
            'filepath': 'a_module',
          },
        },
        'WATCHLISTS': {
          'a_module': %s,
        },
      } """ % watchers
        watchlists.Watchlists._HasWatchlistsFile.return_value = True
        watchlists.Watchlists._ContentsOfWatchlistsFile.return_value = contents

        wl = watchlists.Watchlists('/a/path')
        self.assertEqual(wl.GetWatchersForPaths(['a_module']), watchers)

    def testMultipleWatchlistsTrigger(self):
        """Test that multiple watchlists can get triggered for one filepath."""
        contents = \
          """{
        'WATCHLIST_DEFINITIONS': {
          'mac': {
            'filepath': 'mac',
          },
          'views': {
            'filepath': 'views',
          },
        },
        'WATCHLISTS': {
          'mac': ['x1@chromium.org'],
          'views': ['x2@chromium.org'],
        },
      } """
        watchlists.Watchlists._HasWatchlistsFile.return_value = True
        watchlists.Watchlists._ContentsOfWatchlistsFile.return_value = contents

        wl = watchlists.Watchlists('/a/path')
        self.assertEqual(wl.GetWatchersForPaths(['file_views_mac']),
                         ['x1@chromium.org', 'x2@chromium.org'])

    def testDuplicateWatchers(self):
        """Test that multiple watchlists can get triggered for one filepath."""
        watchers = ['someone@chromium.org']
        contents = \
          """{
        'WATCHLIST_DEFINITIONS': {
          'mac': {
            'filepath': 'mac',
          },
          'views': {
            'filepath': 'views',
          },
        },
        'WATCHLISTS': {
          'mac': %s,
          'views': %s,
        },
      } """ % (watchers, watchers)
        watchlists.Watchlists._HasWatchlistsFile.return_value = True
        watchlists.Watchlists._ContentsOfWatchlistsFile.return_value = contents

        wl = watchlists.Watchlists('/a/path')
        self.assertEqual(wl.GetWatchersForPaths(['file_views_mac']), watchers)

    def testWinPathWatchers(self):
        """Test watchers for a windows path (containing backward slashes)."""
        watchers = ['abc@def.com', 'x1@xyz.org']
        contents = \
          """{
        'WATCHLIST_DEFINITIONS': {
          'browser': {
            'filepath': 'chrome/browser/.*',
          },
        },
        'WATCHLISTS': {
          'browser': %s,
        },
      } """ % watchers
        saved_sep = watchlists.os.sep
        watchlists.os.sep = '\\'  # to pose as win32
        watchlists.Watchlists._HasWatchlistsFile.return_value = True
        watchlists.Watchlists._ContentsOfWatchlistsFile.return_value = contents

        wl = watchlists.Watchlists(r'a\path')
        returned_watchers = wl.GetWatchersForPaths(
            [r'chrome\browser\renderer_host\render_widget_host.h'])
        watchlists.os.sep = saved_sep  # revert back os.sep before asserts
        self.assertEqual(returned_watchers, watchers)


if __name__ == '__main__':
    import unittest
    unittest.main()
