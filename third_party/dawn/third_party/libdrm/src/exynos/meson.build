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

libdrm_exynos = library(
  'drm_exynos',
  [files('exynos_drm.c', 'exynos_fimg2d.c'), config_file],
  c_args : libdrm_c_args,
  gnu_symbol_visibility : 'hidden',
  include_directories : [inc_root, inc_drm],
  link_with : libdrm,
  dependencies : [dep_threads],
  version : '1.0.0',
  install : true,
)

install_headers('exynos_drmif.h', subdir : 'libdrm')
install_headers('exynos_drm.h', 'exynos_fimg2d.h', subdir : 'exynos')

ext_libdrm_exynos = declare_dependency(
  link_with : [libdrm, libdrm_exynos],
  include_directories : [inc_drm, include_directories('.')],
)

if meson.version().version_compare('>= 0.54.0')
  meson.override_dependency('libdrm_exynos', ext_libdrm_exynos)
endif

pkg.generate(
  libdrm_exynos,
  name : 'libdrm_exynos',
  subdirs : ['.', 'libdrm', 'exynos'],
  version : '0.7',
  description : 'Userspace interface to exynos kernel DRM services',
)

test(
  'exynos-symbols-check',
  symbols_check,
  args : [
    '--lib', libdrm_exynos,
    '--symbols-file', files('exynos-symbols.txt'),
    '--nm', prog_nm.full_path(),
  ],
)
