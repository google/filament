# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""App Engine config.

This module is loaded before others and can be used to set up the
App Engine environment. See:
  https://cloud.google.com/appengine/docs/python/tools/appengineconfig
"""

import os

from google.appengine.ext import vendor

appstats_SHELL_OK = True

# Directories in catapult/third_party required by uploader/corpus cleanup.
THIRD_PARTY_LIBRARIES = [
    'apiclient',
    'cloudstorage',
    'uritemplate',
]
# Libraries bundled with the App Engine SDK.
THIRD_PARTY_LIBRARIES_IN_SDK = [
    'httplib2',
    'oauth2client',
    'six',
]

def _AddThirdPartyLibraries():
  """Registers the third party libraries with App Engine.

  In order for third-party libraries to be available in the App Engine
  runtime environment, they must be added with vendor.add. The directories
  added this way must be inside the App Engine project directory.
  """
  # The deploy script is expected to add links to third party libraries
  # before deploying. If the directories aren't there (e.g. when running tests)
  # then just ignore it.
  for library_dir in (THIRD_PARTY_LIBRARIES +
                      THIRD_PARTY_LIBRARIES_IN_SDK):
    if os.path.exists(library_dir):
      vendor.add(os.path.join(os.path.dirname(__file__), library_dir))


_AddThirdPartyLibraries()
