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

#include "dawn/replay/Capture.h"

#include <span>
#include <sstream>
#include <utility>

namespace dawn::replay {

Capture::~Capture() = default;

std::unique_ptr<Capture> Capture::Create(CaptureStream& commandStream,
                                         size_t commandSize,
                                         CaptureStream& contentStream,
                                         size_t contentSize) {
    return CaptureImpl::Create(commandStream, commandSize, contentStream, contentSize);
}

std::unique_ptr<CaptureImpl> CaptureImpl::Create(CaptureStream& commandStream,
                                                 size_t commandSize,
                                                 CaptureStream& contentStream,
                                                 size_t contentSize) {
    std::vector<uint8_t> commands;
    commands.resize(commandSize);
    commandStream.read(reinterpret_cast<char*>(commands.data()), commandSize);

    std::vector<uint8_t> content;
    content.resize(contentSize);
    contentStream.read(reinterpret_cast<char*>(content.data()), contentSize);

    return std::unique_ptr<CaptureImpl>(new CaptureImpl(std::move(commands), std::move(content)));
}

CaptureImpl::CaptureImpl(std::vector<uint8_t> commands, std::vector<uint8_t> content)
    : mCommands(std::move(commands)), mContent(std::move(content)) {}

CaptureImpl::~CaptureImpl() {}

bool CaptureImpl::Walk(RootCommandVisitor& visitor) {
    return CaptureWalker::Walk(visitor).IsSuccess();
}

ReadHead CaptureImpl::GetCommandReadHead() const {
    return ReadHead(mCommands);
}

ReadHead CaptureImpl::GetContentReadHead() const {
    return ReadHead(mContent);
}

}  // namespace dawn::replay
