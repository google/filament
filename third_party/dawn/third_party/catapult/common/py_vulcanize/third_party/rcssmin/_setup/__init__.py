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
 Package _setup
================

This package provides tools for main package setup.
"""
__author__ = "Andr\xe9 Malo"
__docformat__ = "restructuredtext en"

import os as _os
import sys as _sys

if _sys.version_info[0] == 2:
    __path__ = [_os.path.join(__path__[0], 'py2')]
    __author__ = __author__.decode('latin-1')
elif _sys.version_info[0] == 3:
    __path__ = [_os.path.join(__path__[0], 'py3')]
else:
    raise RuntimeError("Unsupported python version")
del _os, _sys

from _setup.setup import run # pylint: disable = W0611
