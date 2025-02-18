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

libdrm_intel = library(
  'drm_intel',
  [
    files(
      'intel_bufmgr.c', 'intel_bufmgr_fake.c', 'intel_bufmgr_gem.c',
      'intel_decode.c', 'mm.c',
    ),
    config_file,
  ],
  include_directories : [inc_root, inc_drm],
  link_with : libdrm,
  dependencies : [dep_pciaccess, dep_threads, dep_rt, dep_valgrind, dep_atomic_ops],
  c_args : libdrm_c_args,
  gnu_symbol_visibility : 'hidden',
  version : '1.0.0',
  install : true,
)

ext_libdrm_intel = declare_dependency(
  link_with : [libdrm, libdrm_intel],
  include_directories : [inc_drm, include_directories('.')],
)

if meson.version().version_compare('>= 0.54.0')
  meson.override_dependency('libdrm_intel', ext_libdrm_intel)
endif

install_headers(
  'intel_bufmgr.h', 'intel_aub.h', 'intel_debug.h',
  subdir : 'libdrm',
)

pkg.generate(
  libdrm_intel,
  name : 'libdrm_intel',
  subdirs : ['.', 'libdrm'],
  requires : 'libdrm',
  description : 'Userspace interface to intel kernel DRM services',
)

test_decode = executable(
  'test_decode',
  files('test_decode.c'),
  include_directories : [inc_root, inc_drm],
  link_with : [libdrm, libdrm_intel],
  c_args : libdrm_c_args,
  gnu_symbol_visibility : 'hidden',
)

test(
  'gen4-3d.batch',
  find_program('tests/gen4-3d.batch.sh'),
  workdir : meson.current_build_dir(),
)
test(
  'gen45-3d.batch',
  find_program('tests/gm45-3d.batch.sh'),
  workdir : meson.current_build_dir(),
)
test(
  'gen5-3d.batch',
  find_program('tests/gen5-3d.batch.sh'),
  workdir : meson.current_build_dir(),
)
test(
  'gen6-3d.batch',
  find_program('tests/gen6-3d.batch.sh'),
  workdir : meson.current_build_dir(),
)
test(
  'gen7-3d.batch',
  find_program('tests/gen7-3d.batch.sh'),
  workdir : meson.current_build_dir(),
)
test(
  'gen7-2d-copy.batch',
  find_program('tests/gen7-2d-copy.batch.sh'),
  workdir : meson.current_build_dir(),
)

test(
  'intel-symbols-check',
  symbols_check,
  args : [
    '--lib', libdrm_intel,
    '--symbols-file', files('intel-symbols.txt'),
    '--nm', prog_nm.full_path(),
  ],
)
