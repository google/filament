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

inc_tegra = include_directories('../../tegra')

libdrm_test = static_library(
  'drm-test',
  [files('drm-test.c', 'drm-test.h'), config_file ],
  include_directories : [inc_root, inc_drm, inc_tegra],
  link_with : libdrm,
)

libdrm_test_tegra = static_library(
  'drm-test-tegra',
  [files(
    'drm-test-tegra.c',
    'drm-test-tegra.h',
    'vic.c',
    'vic.h',
    'vic30.c',
    'vic30.h',
    'vic40.c',
    'vic40.h',
    'vic41.c',
    'vic41.h',
    'vic42.c',
    'vic42.h',
  ), config_file ],
  include_directories : [inc_root, inc_drm, inc_tegra],
  link_with : libdrm,
)

openclose = executable(
  'tegra-openclose',
  files('openclose.c'),
  include_directories : [inc_root, inc_drm, inc_tegra],
  c_args : libdrm_c_args,
  link_with : [libdrm, libdrm_tegra],
  install : with_install_tests,
)

gr2d_fill = executable(
  'tegra-gr2d-fill',
  files('gr2d-fill.c'),
  include_directories : [inc_root, inc_drm, inc_tegra],
  c_args : libdrm_c_args,
  link_with : [libdrm, libdrm_tegra, libdrm_test, libdrm_test_tegra],
  install : with_install_tests,
)

syncpt_wait = executable(
  'tegra-syncpt-wait',
  files('syncpt-wait.c'),
  include_directories : [inc_root, inc_drm, inc_tegra],
  c_args : libdrm_c_args,
  link_with : [libdrm, libdrm_tegra, libdrm_test, libdrm_test_tegra],
  install : with_install_tests,
)

syncpt_timeout = executable(
  'tegra-syncpt-timeout',
  files('syncpt-timeout.c'),
  include_directories : [inc_root, inc_drm, inc_tegra],
  c_args : libdrm_c_args,
  link_with : [libdrm, libdrm_tegra, libdrm_test, libdrm_test_tegra],
  install : with_install_tests,
)

vic_clear = executable(
  'tegra-vic-clear',
  files('vic-clear.c'),
  include_directories : [inc_root, inc_drm, inc_tegra],
  c_args : libdrm_c_args,
  link_with : [libdrm, libdrm_tegra, libdrm_test, libdrm_test_tegra],
  install : with_install_tests,
)

vic_blit = executable(
  'tegra-vic-blit',
  files('vic-blit.c'),
  include_directories : [inc_root, inc_drm, inc_tegra],
  c_args : libdrm_c_args,
  link_with : [libdrm, libdrm_tegra, libdrm_test, libdrm_test_tegra],
  install : with_install_tests,
)

vic_flip = executable(
  'tegra-vic-flip',
  files('vic-flip.c'),
  include_directories : [inc_root, inc_drm, inc_tegra],
  c_args : libdrm_c_args,
  link_with : [libdrm, libdrm_tegra, libdrm_test, libdrm_test_tegra],
  install : with_install_tests,
)
