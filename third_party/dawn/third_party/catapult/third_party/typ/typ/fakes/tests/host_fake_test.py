# Copyright 2014 Dirk Pranke. All rights reserved.
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

from typ.tests import host_test
from typ.fakes.host_fake import FakeHost, FakeResponse

is_python3 = bool(sys.version_info.major == 3)

if is_python3:  # pragma: python3
    # redefining built-in 'unicode' pylint: disable=W0622
    unicode = str

class TestFakeHost(host_test.TestHost):

    def host(self):
        return FakeHost()

    def test_add_to_path(self):
        # TODO: FakeHost uses the real sys.path, and then gets
        # confused becayse host.abspath() doesn't work right for
        # windows-style paths.
        if sys.platform != 'win32':
            super(TestFakeHost, self).test_add_to_path()

    def test_call(self):
        h = self.host()
        ret, out, err = h.call(['echo', 'hello, world'])
        self.assertEqual(ret, 0)
        self.assertEqual(out, '')
        self.assertEqual(err, '')
        self.assertEqual(h.cmds, [['echo', 'hello, world']])

    def test_call_inline(self):
        h = self.host()
        ret = h.call_inline(['echo', 'hello, world'])
        self.assertEqual(ret, 0)

    def test_capture_output(self):
        h = self.host()
        self.host = lambda: h
        super(TestFakeHost, self).test_capture_output()

        # This tests that the super-method only tested the
        # divert=True case, and things were diverted properly.
        self.assertEqual(h.stdout.getvalue(), '')
        self.assertEqual(h.stderr.getvalue(), '')

        h.capture_output(divert=False)
        h.print_('on stdout')
        h.print_('on stderr', stream=h.stderr)
        out, err = h.restore_output()
        self.assertEqual(out, 'on stdout\n')
        self.assertEqual(err, 'on stderr\n')
        self.assertEqual(h.stdout.getvalue(), 'on stdout\n')
        self.assertEqual(h.stderr.getvalue(), 'on stderr\n')

    def test_for_mp(self):
        h = self.host()
        self.assertNotEqual(h.for_mp(), None)

    def test_fetch(self):
        h = self.host()
        url = 'http://localhost/test'
        resp = FakeResponse(unicode('foo'), url)
        h.fetch_responses[url] = resp
        actual_resp = h.fetch(url)
        self.assertEqual(actual_resp.geturl(), url)
        self.assertEqual(actual_resp.getcode(), 200)
        self.assertEqual(resp, actual_resp)
        self.assertEqual(h.fetches, [(url, None, None, actual_resp)])
