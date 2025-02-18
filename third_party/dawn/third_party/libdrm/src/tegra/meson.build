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

libdrm_tegra = library(
  'drm_tegra',
  [
    files(
      'channel.c', 'job.c', 'private.h', 'pushbuf.c', 'syncpt.c', 'tegra.c'
    ),
    config_file
  ],
  include_directories : [inc_root, inc_drm],
  link_with : libdrm,
  dependencies : [dep_threads, dep_atomic_ops],
  c_args : libdrm_c_args,
  gnu_symbol_visibility : 'hidden',
  version : '0.0.0',
  install : true,
)

ext_libdrm_tegra = declare_dependency(
  link_with : [libdrm, libdrm_tegra],
  include_directories : [inc_drm, include_directories('.')],
)

if meson.version().version_compare('>= 0.54.0')
  meson.override_dependency('libdrm_tegra', ext_libdrm_tegra)
endif

install_headers('tegra.h', subdir : 'libdrm')

pkg.generate(
  libdrm_tegra,
  name : 'libdrm_tegra',
  subdirs : ['.', 'libdrm'],
  description : 'Userspace interface to Tegra kernel DRM services',
)

test(
  'tegra-symbols-check',
  symbols_check,
  args : [
    '--lib', libdrm_tegra,
    '--symbols-file', files('tegra-symbols.txt'),
    '--nm', prog_nm.full_path(),
  ],
)
