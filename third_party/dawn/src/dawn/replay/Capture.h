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

#ifndef SRC_DAWN_REPLAY_CAPTURE_H_
#define SRC_DAWN_REPLAY_CAPTURE_H_

#include <istream>
#include <memory>
#include <vector>

#include "dawn/replay/ReadHead.h"
#include "dawn/replay/Replay.h"
#include "src/dawn/replay/CaptureWalker.h"

namespace dawn::replay {

// For now we just expect to load the entire capture into memory.
// In the future we'd expect to be able to stream it though we may
// have to scan it once to find all the commands for a UI.
class CaptureImpl : public Capture, public CaptureWalker {
  public:
    static std::unique_ptr<CaptureImpl> Create(CaptureStream& commandStream,
                                               size_t commandSize,
                                               CaptureStream& contentStream,
                                               size_t contentSize);
    ~CaptureImpl() override;

    bool Walk(RootCommandVisitor& visitor) override;

  protected:
    ReadHead GetCommandReadHead() const override;
    ReadHead GetContentReadHead() const override;

  private:
    CaptureImpl(std::vector<uint8_t> commands, std::vector<uint8_t> content);

    std::vector<uint8_t> mCommands;
    std::vector<uint8_t> mContent;
};

}  // namespace dawn::replay

#endif  // SRC_DAWN_REPLAY_CAPTURE_H_
