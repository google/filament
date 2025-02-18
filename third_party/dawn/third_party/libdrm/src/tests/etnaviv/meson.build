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

inc_etnaviv_tests = [inc_root, inc_drm, include_directories('../../etnaviv')]

etnaviv_2d_test = executable(
  'etnaviv_2d_test',
  files('etnaviv_2d_test.c', 'write_bmp.c'),
  include_directories : inc_etnaviv_tests,
  link_with : [libdrm, libdrm_etnaviv],
  install : with_install_tests,
)

etnaviv_cmd_stream_test = executable(
  'etnaviv_cmd_stream_test',
  files('etnaviv_cmd_stream_test.c'),
  include_directories : inc_etnaviv_tests,
  link_with : [libdrm, libdrm_etnaviv],
  install : with_install_tests,
)

etnaviv_bo_cache_test = executable(
  'etnaviv_bo_cache_test',
  files('etnaviv_bo_cache_test.c'),
  include_directories : inc_etnaviv_tests,
  link_with : [libdrm, libdrm_etnaviv],
  install : with_install_tests,
)
