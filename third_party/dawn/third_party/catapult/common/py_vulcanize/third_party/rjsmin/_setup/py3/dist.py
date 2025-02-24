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
================
 dist utilities
================

dist utilities.
"""
__author__ = "Andr\xe9 Malo"
__docformat__ = "restructuredtext en"

import sys as _sys

from _setup import shell as _shell


def run_setup(*args, **kwargs):
    """ Run setup """
    if 'setup' in kwargs:
        script = kwargs.get('setup') or 'setup.py'
        del kwargs['setup']
    else:
        script = 'setup.py'
    if 'fakeroot' in kwargs:
        fakeroot = kwargs['fakeroot']
        del kwargs['fakeroot']
    else:
        fakeroot = None
    if kwargs:
        raise TypeError("Unrecognized keyword parameters")

    script = _shell.native(script)
    argv = [_sys.executable, script] + list(args)
    if fakeroot:
        argv.insert(0, fakeroot)
    return not _shell.spawn(*argv)
