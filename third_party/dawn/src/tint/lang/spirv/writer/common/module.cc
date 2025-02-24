// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/common/module.h"

namespace tint::spirv::writer {
namespace {

/// Helper to return the size in words of an instruction list when serialized.
/// @param instructions the instruction list
/// @returns the number of words needed to serialize the list
uint32_t SizeOf(const InstructionList& instructions) {
    uint32_t size = 0;
    for (const auto& inst : instructions) {
        size += inst.WordLength();
    }
    return size;
}

}  // namespace

Module::Module() = default;

Module::Module(const Module&) = default;

Module::Module(Module&&) = default;

Module::~Module() = default;

Module& Module::operator=(const Module& other) = default;

Module& Module::operator=(Module&& other) = default;

uint32_t Module::TotalSize() const {
    // The 5 covers the magic, version, generator, id bound and reserved.
    uint32_t size = 5;

    size += SizeOf(capabilities_);
    size += SizeOf(extensions_);
    size += SizeOf(ext_imports_);
    size += SizeOf(memory_model_);
    size += SizeOf(entry_points_);
    size += SizeOf(execution_modes_);
    size += SizeOf(debug_);
    size += SizeOf(annotations_);
    size += SizeOf(types_);
    for (const auto& func : functions_) {
        size += func.WordLength();
    }

    return size;
}

void Module::Iterate(std::function<void(const Instruction&)> cb) const {
    for (const auto& inst : capabilities_) {
        cb(inst);
    }
    for (const auto& inst : extensions_) {
        cb(inst);
    }
    for (const auto& inst : ext_imports_) {
        cb(inst);
    }
    for (const auto& inst : memory_model_) {
        cb(inst);
    }
    for (const auto& inst : entry_points_) {
        cb(inst);
    }
    for (const auto& inst : execution_modes_) {
        cb(inst);
    }
    for (const auto& inst : debug_) {
        cb(inst);
    }
    for (const auto& inst : annotations_) {
        cb(inst);
    }
    for (const auto& inst : types_) {
        cb(inst);
    }
    for (const auto& func : functions_) {
        func.Iterate(cb);
    }
}

void Module::PushCapability(uint32_t cap) {
    if (capability_set_.Add(cap)) {
        capabilities_.push_back(Instruction{spv::Op::OpCapability, {Operand(cap)}});
    }
}

void Module::PushExtension(const char* extension) {
    if (extension_set_.Add(extension)) {
        extensions_.push_back(Instruction{spv::Op::OpExtension, {Operand(extension)}});
    }
}

}  // namespace tint::spirv::writer
