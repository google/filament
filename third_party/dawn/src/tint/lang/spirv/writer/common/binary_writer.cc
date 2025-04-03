// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/common/binary_writer.h"

#include <cstring>
#include <string>

#include "src/tint/utils/memory/bitcast.h"

namespace tint::spirv::writer {
namespace {

const uint32_t kGeneratorId = 23u << 16;

}  // namespace

BinaryWriter::BinaryWriter() = default;

BinaryWriter::~BinaryWriter() = default;

void BinaryWriter::WriteModule(const Module& module) {
    out_.reserve(module.TotalSize());
    module.Iterate([this](const Instruction& inst) { this->ProcessInstruction(inst); });
}

void BinaryWriter::WriteInstruction(const Instruction& inst) {
    ProcessInstruction(inst);
}

void BinaryWriter::WriteHeader(uint32_t bound, uint32_t version) {
    out_.push_back(spv::MagicNumber);
    out_.push_back(0x00010300);  // Version 1.3
    out_.push_back(kGeneratorId | version);
    out_.push_back(bound);
    out_.push_back(0);
}

void BinaryWriter::ProcessInstruction(const Instruction& inst) {
    TINT_ASSERT(inst.WordLength() < 65536);
    out_.push_back(inst.WordLength() << 16 | static_cast<uint32_t>(inst.Opcode()));
    for (const auto& op : inst.Operands()) {
        ProcessOp(op);
    }
}

void BinaryWriter::ProcessOp(const Operand& op) {
    if (auto* i = std::get_if<uint32_t>(&op)) {
        out_.push_back(*i);
        return;
    }
    if (auto* f = std::get_if<float>(&op)) {
        out_.push_back(tint::Bitcast<uint32_t>(*f));
        return;
    }
    if (auto* str = std::get_if<std::string>(&op)) {
        auto idx = out_.size();
        out_.resize(out_.size() + OperandLength(op), 0);
        memcpy(&out_[idx], str->c_str(), str->size() + 1);
        return;
    }
}

}  // namespace tint::spirv::writer
