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

"""App Engine py.test configuration."""

import sys

from six.moves import reload_module


def set_up_gae_environment(sdk_path):
    """Set up appengine SDK third-party imports.

    The App Engine SDK does terrible things to the global interpreter state.
    Because of this, this stuff can't be neatly undone. As such, it can't be
    a fixture.
    """
    if 'google' in sys.modules:
        # Some packages, such as protobuf, clobber the google
        # namespace package. This prevents that.
        reload_module(sys.modules['google'])

    # This sets up google-provided libraries.
    sys.path.insert(0, sdk_path)
    import dev_appserver
    dev_appserver.fix_sys_path()

    # Fixes timezone and other os-level items.
    import google.appengine.tools.os_compat  # noqa: unused import


def pytest_configure(config):
    """Configures the App Engine SDK imports on py.test startup."""
    if config.getoption('gae_sdk') is not None:
        set_up_gae_environment(config.getoption('gae_sdk'))


def pytest_ignore_collect(path, config):
    """Skip App Engine tests when --gae-sdk is not specified."""
    return (
        'contrib/appengine' in str(path) and
        config.getoption('gae_sdk') is None)
