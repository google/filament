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

import unittest

from typ.stats import Stats


class TestStats(unittest.TestCase):

    def test_basic(self):
        s = Stats('foo', lambda: 0, 32)
        self.assertEqual(s.format(), 'foo')

    def test_edges(self):
        s = Stats('[%s/%f/%t/%r/%p]', lambda: 0, 32)
        self.assertEqual(s.format(), '[0/0/0/0/-]')
        s.started = 3
        s.total = 5
        s.finished = 1
        self.assertEqual(s.format(), '[3/1/5/2/ 60.0]')

        s.started = 5
        s.finished = 5
        self.assertEqual(s.format(), '[5/5/5/0/100.0]')

    def test_elapsed_time(self):
        times = [0.0, 0.4]
        s = Stats('[%e]', lambda: times.pop(0), 32)
        self.assertEqual(s.format(), '[0.400]')

        s = Stats('[%e]', lambda: 0, 32)
        self.assertEqual(s.format(), '[0.000]')

    def test_current_rate(self):
        times = [0.0, 0.1, 0.2]
        s = Stats('[%c]', lambda: times.pop(0), 1)
        self.assertEquals(s.format(), '[-]')
        s.add_time()
        s.add_time()
        self.assertEquals(s.format(), '[ 10.0]')

    def test_overall_rate(self):
        times = [0, 0, 5]
        s = Stats('[%o]', lambda: times.pop(0), 32)
        self.assertEqual(s.format(), '[-]')
        s.started = 3
        s.finished = 1
        s.total = 5
        self.assertEqual(s.format(), '[  0.2]')

    def test_escaped_percent(self):
        s = Stats('%%', lambda: 0, 32)
        self.assertEqual(s.format(), '%')

    def test_unrecognized_escape(self):
        s = Stats('%x', lambda: 0, 32)
        self.assertEqual(s.format(), '%x')

    def test_remaining(self):
        s = Stats('%u', lambda: 0, 32)
        s.total = 2
        self.assertEqual(s.format(), '2')
