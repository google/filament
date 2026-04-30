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

set(headers
    "${DAWN_INCLUDE_DIR}/dawn/replay/Replay.h"
    "${DAWN_INCLUDE_DIR}/dawn/replay/dawn_replay_export.h"
)

set(private_headers
    "BlitBufferToDepthTexture.h"
    "Capture.h"
    "Deserialization.h"
    "Error.h"
    "ReadHead.h"
    "ReplayImpl.h"
    "CaptureWalker.h"
)

set(sources
    "BlitBufferToDepthTexture.cpp"
    "Capture.cpp"
    "Deserialization.cpp"
    "Error.cpp"
    "ReadHead.cpp"
    "Replay.cpp"
    "CaptureWalker.cpp"
)

dawn_add_library(
    replay
    UTILITY_TARGET dawn_internal_config
    HEADERS
        ${headers}
    PRIVATE_HEADERS
        ${private_headers}
    SOURCES
        ${sources}
    DEPENDS
        dawncpp_headers
        dawn_proc
        dawn_common
    PRIVATE_DEPENDS
        absl::flat_hash_map
        absl::str_format
)
target_compile_definitions(replay PRIVATE "DAWN_REPLAY_IMPLEMENTATION")
if(BUILD_SHARED_LIBS)
    target_compile_definitions(replay PUBLIC "DAWN_REPLAY_SHARED_LIBRARY")
endif()
