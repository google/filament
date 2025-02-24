# -*- coding: ascii -*-
#
# Copyright 2007, 2008, 2009, 2010, 2011
# Andr\xe9 Malo or his licensors, as applicable
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""
=================
 Setup utilities
=================

Setup utilities.
"""
from __future__ import print_function
__author__ = u"Andr\xe9 Malo"
__docformat__ = "restructuredtext en"

try:
    from distutils import log
except ImportError:
    class log(object):
        def info(self, value):
            print(value)
        def debug(self, value):
            pass
    log = log()

from distutils import util as _util
try:
    from ConfigParser import SafeConfigParser
except ImportError:
    import ConfigParser as _config_parser
    class SafeConfigParser(_config_parser.ConfigParser):
        """ Safe config parser """
        def _interpolate(self, section, option, rawval, vars):
            return rawval

        def items(self, section):
            return [(key, self.get(section, key))
                for key in self.options(section)
            ]


def humanbool(name, value):
    """
    Determine human boolean value

    :Parameters:
      `name` : ``str``
        The config key (used for error message)

      `value` : ``str``
        The config value

    :Return: The boolean value
    :Rtype: ``bool``

    :Exceptions:
      - `ValueError` : The value could not be recognized
    """
    try:
        return _util.strtobool(str(value).strip().lower() or 'no')
    except ValueError:
        raise ValueError("Unrecognized config value: %s = %s" % (name, value))
