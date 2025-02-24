#!/usr/bin/env python
#
# Copyright 2022 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os

webgpu_cts_scripts_dir = os.path.dirname(os.path.abspath(__file__))
dawn_dir = os.path.dirname(os.path.dirname(webgpu_cts_scripts_dir))
dawn_third_party_dir = os.path.join(dawn_dir, 'third_party')
gn_webgpu_cts_dir = os.path.join(dawn_third_party_dir, 'gn', 'webgpu-cts')
webgpu_cts_root_dir = os.path.join(dawn_third_party_dir, 'webgpu-cts')
chromium_third_party_dir = None
node_dir = None

_possible_chromium_third_party_dir = os.path.dirname(
    os.path.dirname(dawn_third_party_dir))
_possible_node_dir = os.path.join(_possible_chromium_third_party_dir, 'node')
if os.path.exists(_possible_node_dir):
    chromium_third_party_dir = _possible_chromium_third_party_dir
    node_dir = _possible_node_dir
