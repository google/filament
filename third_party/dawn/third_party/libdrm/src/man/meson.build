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

rst_pages = [
  ['drm', '7'],
  ['drm-kms', '7'],
  ['drm-memory', '7'],
  ['drmAvailable', '3'],
  ['drmHandleEvent', '3'],
  ['drmModeGetResources', '3'],
]
foreach page : rst_pages
  name = page[0] + '.' + page[1]
  rst = files(name + '.rst')
  custom_target(
    name,
    input : rst,
    output : name,
    command : [prog_rst2man, '@INPUT@', '@OUTPUT@'],
    install : true,
    install_dir : join_paths(get_option('mandir'), 'man' + page[1]),
  )
endforeach
