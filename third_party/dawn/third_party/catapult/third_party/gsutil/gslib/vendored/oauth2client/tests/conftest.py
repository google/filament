# Copyright 2016 Google Inc. All rights reserved.
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

"""Py.test hooks."""

from oauth2client import _helpers


def pytest_addoption(parser):
    """Adds the --gae-sdk option to py.test.

    This is used to enable the GAE tests. This has to be in this conftest.py
    due to the way py.test collects conftest files."""
    parser.addoption('--gae-sdk')


def pytest_configure(config):
    """Py.test hook called before loading tests."""
    # Default of POSITIONAL_WARNING is too verbose for testing
    _helpers.positional_parameters_enforcement = _helpers.POSITIONAL_EXCEPTION
