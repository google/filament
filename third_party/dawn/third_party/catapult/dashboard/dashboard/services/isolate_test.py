# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import base64
import unittest
import zlib

from unittest import mock

from dashboard.services import isolate

_FILE_HASH = 'c6911f39564106542b28081c81bde61c43121bda'
_ISOLATED_HASH = 'fc5e63011ae25b057b3097eba4413fc357c05cff'
_FILE_CONTENTS_COMPRESSED = zlib.compress(b'file contents')


@mock.patch('dashboard.services.request.Request')
@mock.patch('dashboard.services.request.RequestJson')
class IsolateServiceTest(unittest.TestCase):

  def testRetrieveUrl(self, request_json, request):
    request_json.return_value = {
        'url':
            'https://isolateserver.storage.googleapis.com/default-gzip/' +
            _FILE_HASH
    }
    request.return_value = _FILE_CONTENTS_COMPRESSED

    file_contents = isolate.Retrieve('https://isolate.com', _FILE_HASH)
    self.assertEqual(file_contents, b'file contents')

    url = 'https://isolate.com/_ah/api/isolateservice/v1/retrieve'
    body = {'namespace': {'namespace': 'default-gzip'}, 'digest': _FILE_HASH}
    request_json.assert_called_once_with(url, 'POST', body)

    request.assert_called_once_with(
        'https://isolateserver.storage.googleapis.com/default-gzip/' +
        _FILE_HASH, 'GET')

  def testRetrieveContent(self, request_json, _):
    request_json.return_value = {
        'content': base64.b64encode(_FILE_CONTENTS_COMPRESSED)
    }

    isolate_contents = isolate.Retrieve('https://isolate.com', _ISOLATED_HASH)
    self.assertEqual(isolate_contents, b'file contents')

    url = 'https://isolate.com/_ah/api/isolateservice/v1/retrieve'
    body = {
        'namespace': {
            'namespace': 'default-gzip'
        },
        'digest': _ISOLATED_HASH,
    }
    request_json.assert_called_once_with(url, 'POST', body)

  def testRetrieveUnknownFormat(self, request_json, _):
    request_json.return_value = {}

    with self.assertRaises(NotImplementedError):
      isolate.Retrieve('https://isolate.com', 'digest')
