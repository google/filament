# Copyright © 2017-2018 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


datadir_amdgpu = join_paths(get_option('prefix'), get_option('datadir'), 'libdrm')

libdrm_amdgpu = library(
  'drm_amdgpu',
  [
    files(
      'amdgpu_asic_id.c', 'amdgpu_bo.c', 'amdgpu_cs.c', 'amdgpu_device.c',
      'amdgpu_gpu_info.c', 'amdgpu_vamgr.c', 'amdgpu_vm.c', 'handle_table.c',
    ),
    config_file,
  ],
  c_args : [
    libdrm_c_args,
    '-DAMDGPU_ASIC_ID_TABLE="@0@"'.format(join_paths(datadir_amdgpu, 'amdgpu.ids')),
  ],
  include_directories : [inc_root, inc_drm],
  link_with : libdrm,
  dependencies : [dep_threads, dep_atomic_ops, dep_rt],
  version : '1.0.0',
  install : true,
)

install_headers('amdgpu.h', subdir : 'libdrm')

pkg.generate(
  libdrm_amdgpu,
  name : 'libdrm_amdgpu',
  subdirs : ['.', 'libdrm'],
  description : 'Userspace interface to kernel DRM services for amdgpu',
)

ext_libdrm_amdgpu = declare_dependency(
  link_with : [libdrm, libdrm_amdgpu],
  include_directories : [inc_drm, include_directories('.')],
)

if meson.version().version_compare('>= 0.54.0')
  meson.override_dependency('libdrm_amdgpu', ext_libdrm_amdgpu)
endif

test(
  'amdgpu-symbols-check',
  symbols_check,
  args : [
    '--lib', libdrm_amdgpu,
    '--symbols-file', files('amdgpu-symbols.txt'),
    '--nm', prog_nm.full_path(),
  ],
)
