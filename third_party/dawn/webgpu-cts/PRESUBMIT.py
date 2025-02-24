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

import sys

PRESUBMIT_VERSION = '2.0.0'


def CheckCtsValidate(input_api, output_api):
    sys.path += [input_api.change.RepositoryRoot()]

    from go_presubmit_support import go_path

    results = []
    try:
        tools_path = input_api.os_path.join(input_api.change.RepositoryRoot(),
                                            'tools')
        cts_bin = input_api.os_path.join(tools_path, 'bin', 'cts')
        if input_api.is_windows:
            cts_bin += '.exe'

        cmd = [
            go_path(input_api), 'build', '-o', cts_bin,
            input_api.os_path.join('.', 'cmd', 'cts')
        ]
        input_api.subprocess.check_call_out(cmd,
                                            stdout=input_api.subprocess.PIPE,
                                            stderr=input_api.subprocess.PIPE,
                                            cwd=input_api.os_path.join(
                                                tools_path, 'src'))

        cmd = [cts_bin, 'validate']
        input_api.subprocess.check_call_out(
            cmd,
            stdout=input_api.subprocess.PIPE,
            stderr=input_api.subprocess.PIPE,
            cwd=input_api.change.RepositoryRoot())
    except input_api.subprocess.CalledProcessError as e:
        results.append(output_api.PresubmitError('%s' % (e, )))
    return results


def CheckHeaderSync(input_api, output_api):
    results = []
    sync_script = input_api.os_path.join(input_api.PresubmitLocalPath(),
                                         'scripts', 'check_headers_in_sync.py')
    try:
        input_api.subprocess.check_call_out([sync_script],
                                            stdout=input_api.subprocess.PIPE,
                                            stderr=input_api.subprocess.STDOUT)
    except input_api.subprocess.CalledProcessError as e:
        results.append(output_api.PresubmitError(str(e)))
    return results
