#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import unittest

from . import symbolize_trace_atos_regex


class AtosRegexTest(unittest.TestCase):
  def testRegex(self):
    if sys.platform != "darwin":
      return
    matcher = symbolize_trace_atos_regex.AtosRegexMatcher()
    text = "-[SKTGraphicView drawRect:] (in Sketch) (SKTGraphicView.m:445)"
    output = matcher.Match(text)
    self.assertEqual("-[SKTGraphicView drawRect:]", output)

    text = "malloc (in libsystem_malloc.dylib) + 42"
    output = matcher.Match(text)
    self.assertEqual("malloc", output)

    expected_output = (
        "content::CacheStorage::MatchAllCaches(std::__1::unique_ptr<content::Se"
        "rviceWorkerFetchRequest, std::__1::default_delete<content::ServiceWork"
        "erFetchRequest> >, content::CacheStorageCacheQueryParams const&, base:"
        ":Callback<void (content::CacheStorageError, std::__1::unique_ptr<conte"
        "nt::ServiceWorkerResponse, std::__1::default_delete<content::ServiceWo"
        "rkerResponse> >, std::__1::unique_ptr<storage::BlobDataHandle, std::__"
        "1::default_delete<storage::BlobDataHandle> >), (base::internal::CopyMo"
        "de)1, (base::internal::RepeatMode)1> const&)"
    )
    text = expected_output + " (in Chromium Framework) (ref_counted.h:322)"
    output = matcher.Match(text)
    self.assertEqual(expected_output, output)

    text = "0x4a12"
    output = matcher.Match(text)
    self.assertEqual(text, output)

    text = "0x00000d9a (in Chromium)"
    output = matcher.Match(text)
    self.assertEqual(text, output)


if __name__ == '__main__':
  unittest.main()
