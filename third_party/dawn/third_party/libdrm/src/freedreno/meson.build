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

files_freedreno = files(
  'freedreno_device.c',
  'freedreno_pipe.c',
  'freedreno_ringbuffer.c',
  'freedreno_bo.c',
  'freedreno_bo_cache.c',
  'msm/msm_bo.c',
  'msm/msm_device.c',
  'msm/msm_pipe.c',
  'msm/msm_ringbuffer.c',
)

if with_freedreno_kgsl
  files_freedreno += files(
    'kgsl/kgsl_bo.c',
    'kgsl/kgsl_device.c',
    'kgsl/kgsl_pipe.c',
    'kgsl/kgsl_ringbuffer.c',
  )
endif

libdrm_freedreno = library(
  'drm_freedreno',
  [files_freedreno, config_file],
  c_args : libdrm_c_args,
  include_directories : [inc_root, inc_drm],
  dependencies : [dep_valgrind, dep_threads, dep_rt, dep_atomic_ops],
  link_with : libdrm,
  version : '1.0.0',
  install : true,
)

ext_libdrm_freedreno = declare_dependency(
  link_with : [libdrm, libdrm_freedreno],
  include_directories : [inc_drm, include_directories('.')],
)

if meson.version().version_compare('>= 0.54.0')
  meson.override_dependency('libdrm_freedreno', ext_libdrm_freedreno)
endif

install_headers(
  'freedreno_drmif.h', 'freedreno_ringbuffer.h',
  subdir : 'freedreno'
)

pkg.generate(
  libdrm_freedreno,
  name : 'libdrm_freedreno',
  subdirs : ['.', 'libdrm', 'freedreno'],
  description : 'Userspace interface to freedreno kernel DRM services',
)

test(
  'freedreno-symbols-check',
  symbols_check,
  args : [
    '--lib', libdrm_freedreno,
    '--symbols-file', files('freedreno-symbols.txt'),
    '--nm', prog_nm.full_path(),
  ],
)
