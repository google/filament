# Copyright © 2017 Intel Corporation

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

libdrm_omap = library(
  'drm_omap',
  [files('omap_drm.c'), config_file],
  include_directories : [inc_root, inc_drm],
  c_args : libdrm_c_args,
  gnu_symbol_visibility : 'hidden',
  link_with : libdrm,
  dependencies : [dep_threads, dep_atomic_ops],
  version : '1.0.0',
  install : true,
)

ext_libdrm_omap = declare_dependency(
  link_with : [libdrm, libdrm_omap],
  include_directories : [inc_drm, include_directories('.')],
)

if meson.version().version_compare('>= 0.54.0')
  meson.override_dependency('libdrm_omap', ext_libdrm_omap)
endif

install_headers('omap_drmif.h', subdir : 'libdrm')
install_headers('omap_drm.h', subdir : 'omap')

pkg.generate(
  libdrm_omap,
  name : 'libdrm_omap',
  subdirs : ['.', 'libdrm', 'omap'],
  version : '0.6',
  description : 'Userspace interface to omap kernel DRM services',
)

test(
  'omap-symbols-check',
  symbols_check,
  args : [
    '--lib', libdrm_omap,
    '--symbols-file', files('omap-symbols.txt'),
    '--nm', prog_nm.full_path(),
  ],
)
