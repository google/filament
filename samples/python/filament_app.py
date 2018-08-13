#!/usr/bin/env python3

import os
import setuptools

__version__ = '0.0.1'


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)


filament_dir = '/home/nan/p/filament/'

includes = [
    'filament',
    'third_party/libsdl2',
    'third_party/imgui',
    'libs/utils',
    'libs/math',
    'libs/filabridge',
]

include_dirs = [
    get_pybind_include(),
    get_pybind_include(user=True)
]

include_dirs += [os.path.join(filament_dir, i, 'include') for i in includes]
print(include_dirs)
