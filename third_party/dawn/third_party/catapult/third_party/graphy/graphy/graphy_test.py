#!/usr/bin/python2.4
#
# Copyright 2008 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Base test code for Graphy."""

import unittest


class GraphyTest(unittest.TestCase):
  """Base class for other Graphy tests."""

  def assertIn(self, a, b, msg=None):
    """Just like self.assert_(a in b), but with a nicer default message."""
    if msg is None:
      msg = '"%s" not found in "%s"' % (a, b)
    self.assert_(a in b, msg)

  def assertNotIn(self, a, b, msg=None):
    """Just like self.assert_(a not in b), but with a nicer default message."""
    if msg is None:
      msg = '"%s" unexpectedly found in "%s"' % (a, b)
    self.assert_(a not in b, msg)

  def Param(self, param_name, chart=None):
    """Helper to look up a Google Chart API parameter for the given chart."""
    if chart is None:
      chart = self.chart
    params = chart.display._Params(chart)
    return params[param_name]

def main():
  """Wrap unittest.main (for convenience of caller)."""
  return unittest.main()
