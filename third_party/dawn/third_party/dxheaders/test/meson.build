# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cpp = meson.get_compiler('cpp')
d3d12_lib = cpp.find_library('d3d12')
dxcore_lib = cpp.find_library('dxcore')

headers_test = executable('DirectX-Headers-Test', 'test.cpp',
    dependencies : [dep_dxheaders, d3d12_lib, dxcore_lib])