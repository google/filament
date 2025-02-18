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

if dep_cunit.found()
  amdgpu_test = executable(
    'amdgpu_test',
    files(
      'amdgpu_test.c', 'basic_tests.c', 'bo_tests.c', 'cs_tests.c',
      'vce_tests.c', 'uvd_enc_tests.c', 'vcn_tests.c', 'deadlock_tests.c',
      'vm_tests.c', 'ras_tests.c', 'syncobj_tests.c', 'security_tests.c',
      'hotunplug_tests.c', 'jpeg_tests.c', 'cp_dma_tests.c', 'shader_test_util.c'
    ),
    dependencies : [dep_cunit, dep_threads, dep_atomic_ops],
    include_directories : [inc_root, inc_drm, include_directories('../../amdgpu')],
    link_with : [libdrm, libdrm_amdgpu],
    install : with_install_tests,
  )
endif

amdgpu_stress = executable(
  'amdgpu_stress',
  files(
    'amdgpu_stress.c'
  ),
  dependencies : [dep_threads, dep_atomic_ops],
  include_directories : [inc_root, inc_drm, include_directories('../../amdgpu')],
  link_with : [libdrm, libdrm_amdgpu],
  install : with_install_tests,
)
