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


def go_bin_path(input_api):
    return input_api.os_path.join(input_api.change.RepositoryRoot(), 'tools',
                                  'golang', 'bin')


def go_path(input_api):
    go = input_api.os_path.join(go_bin_path(input_api), 'go')
    if input_api.is_windows:
        go += '.exe'

    return go


def gofmt_path(input_api):
    gofmt = input_api.os_path.join(go_bin_path(input_api), 'gofmt')
    if input_api.is_windows:
        gofmt += '.exe'
    return gofmt


def RunGoTests(input_api, output_api):
    results = []
    try:
        input_api.subprocess.check_call_out(
            [go_path(input_api), 'test', './...'],
            stdout=input_api.subprocess.PIPE,
            stderr=input_api.subprocess.PIPE,
            cwd=input_api.PresubmitLocalPath())
    except input_api.subprocess.CalledProcessError as e:
        results.append(output_api.PresubmitError('%s' % (e, )))
    return results


def EnforceGoFormatting(path, input_api, output_api):
    errors = []
    try:
        stdout, _ = input_api.subprocess.check_call_out(
            [gofmt_path(input_api), '-l', path],
            stdout=input_api.subprocess.PIPE,
            stderr=input_api.subprocess.STDOUT,
            cwd=input_api.PresubmitLocalPath(),
            text=True)
        stdout = stdout.strip()
        if stdout:
            full_path = input_api.os_path.join(input_api.PresubmitLocalPath(),
                                               path)
            errors.append(
                output_api.PresubmitError(
                    'Go code in %s is not formatted, please run "gofmt -w %s"'
                    % (path, full_path)))
    except input_api.subprocess.CalledProcessError as e:
        errors.append(output_api.PresubmitError('EnforceGoFormatting: %s' % e))
    return errors
