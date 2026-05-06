// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef INCLUDE_DAWN_REPLAY_REPLAY_H_
#define INCLUDE_DAWN_REPLAY_REPLAY_H_

#include <istream>
#include <memory>
#include <string_view>

#include "dawn/replay/dawn_replay_export.h"
#include "dawn/webgpu_cpp.h"

namespace dawn::replay {

using CaptureStream = std::istream;

class RootCommandVisitor;

// The public API of a Capture.
// In the future it should have calls to get information (e.g. number of commands)
class DAWN_REPLAY_EXPORT Capture {
  public:
    static std::unique_ptr<Capture> Create(CaptureStream& commandStream,
                                           size_t commandSize,
                                           CaptureStream& contentStream,
                                           size_t contentSize);
    virtual ~Capture() = 0;

    // Returns true if walk successful.
    virtual bool Walk(RootCommandVisitor& visitor) = 0;
};

// The public API of a replay controller of a capture.
// In the future it should have a finer-grained control calls of a capture (e.g. step, play to a
// certain point)
class DAWN_REPLAY_EXPORT Replay {
  public:
    static std::unique_ptr<Replay> Create(wgpu::Device device, std::unique_ptr<Capture> capture);
    virtual ~Replay() = 0;

    template <typename T>
    T GetObjectByLabel(std::string_view label) const;

    // Returns if play is successful.
    bool Play();
};

}  // namespace dawn::replay

#endif  // INCLUDE_DAWN_REPLAY_REPLAY_H_
