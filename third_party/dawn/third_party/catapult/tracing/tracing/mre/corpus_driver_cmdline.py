# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from tracing.mre import local_directory_corpus_driver


_CORPUS_DRIVERS = {
    'local-directory': {
        'description': 'Use traces from a local directory.',
        'class': local_directory_corpus_driver.LocalDirectoryCorpusDriver
    }
}
_CORPUS_DRIVER_DEFAULT = 'local-directory'


def GetCorpusDriver(parser, args):
  # With parse_known_args, optional arguments aren't guaranteed to be there so
  # we need to check if it's there, and use the default otherwise.
  corpus = _CORPUS_DRIVER_DEFAULT

  cls = _CORPUS_DRIVERS[corpus]['class']
  init_args = cls.CheckAndCreateInitArguments(parser, args)
  return cls(**init_args)
