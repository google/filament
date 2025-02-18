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


libdrm_nouveau = library(
  'drm_nouveau',
  [files( 'nouveau.c', 'pushbuf.c', 'bufctx.c', 'abi16.c'), config_file],
  c_args : libdrm_c_args,
  gnu_symbol_visibility : 'hidden',
  include_directories : [inc_root, inc_drm],
  link_with : libdrm,
  dependencies : [dep_threads, dep_atomic_ops],
  version : '2.0.0',
  install : true,
)

ext_libdrm_nouveau = declare_dependency(
  link_with : [libdrm, libdrm_nouveau],
  include_directories : [inc_drm, include_directories('.')],
)

if meson.version().version_compare('>= 0.54.0')
  meson.override_dependency('libdrm_nouveau', ext_libdrm_nouveau)
endif

install_headers('nouveau.h', subdir : 'libdrm/nouveau')
install_headers(
  'nvif/class.h', 'nvif/cl0080.h', 'nvif/cl9097.h', 'nvif/if0002.h',
  'nvif/if0003.h', 'nvif/ioctl.h', 'nvif/unpack.h',
  subdir : 'libdrm/nouveau/nvif'
)

pkg.generate(
  libdrm_nouveau,
  name : 'libdrm_nouveau',
  subdirs : ['.', 'libdrm', 'libdrm/nouveau'],
  description : 'Userspace interface to nouveau kernel DRM services',
)

test(
  'nouveau-symbols-check',
  symbols_check,
  args : [
    '--lib', libdrm_nouveau,
    '--symbols-file', files('nouveau-symbols.txt'),
    '--nm', prog_nm.full_path(),
  ],
)
