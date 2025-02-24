#!/usr/bin/env python
# -*- coding: ascii -*-
#
# Copyright 2006 - 2013
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

import sys as _sys
from _setup import run


def setup(args=None, _manifest=0):
    """ Main setup function """
    from _setup.ext import Extension

    if 'java' in _sys.platform.lower():
        # no c extension for jython
        ext = None
    else:
        ext=[Extension('_rcssmin', sources=['rcssmin.c'])]

    return run(script_args=args, ext=ext, manifest_only=_manifest)


def manifest():
    """ Create List of packaged files """
    return setup((), _manifest=1)


if __name__ == '__main__':
    setup()
