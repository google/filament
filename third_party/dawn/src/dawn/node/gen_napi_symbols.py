#!/usr/bin/env python3

# Copyright 2025 The Dawn & Tint Authors
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
import sys
import re
from pathlib import Path

symbols_js_file = sys.argv[1]
output_file = Path(sys.argv[2])

with open(symbols_js_file, "r") as f:
    matches = re.findall(r"napi_[a-z0-9_]*", f.read())

if os.name == 'nt':
    # Generate the NapiSymbols.def file from the Napi symbol list
    assert (output_file.suffix == ".def")
    with open(output_file, "w") as f:
        f.write("LIBRARY node.exe\n")
        f.write("EXPORTS\n")
        for symbol in matches:
            f.write(f"  {symbol}\n")
else:
    # Generate the NapiSymbols.h file from the Napi symbol list
    assert (output_file.suffix == ".h")
    with open(output_file, "w") as f:
        matches2 = [f"NAPI_SYMBOL({symbol})" for symbol in matches]
        f.write("\n".join(matches2))
