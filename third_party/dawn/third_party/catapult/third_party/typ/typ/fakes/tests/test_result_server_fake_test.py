# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import unittest

from typ.fakes import test_result_server_fake
from typ import Host


class TestResultServerFakeTest(unittest.TestCase):
    def test_basic_upload(self):
        # TODO(crbug.com/1217853) Figure out why this isn't working under
        # py3 (and/or possibly running in parallel on mac).
        if sys.platform in ('darwin', 'win32'):
            return

        host = Host()
        server = None
        posts = []
        try:
            server = test_result_server_fake.start()
            url = 'http://%s:%d/testfile/upload' % server.server_address
            if server:
                resp = host.fetch(url, 'foo=bar')
        finally:
            if server:
                posts = server.stop()
        self.assertEqual(posts, [('post', '/testfile/upload',
                                  'foo=bar'.encode('utf8'))])
        self.assertNotEqual(server.log.getvalue(), '')
