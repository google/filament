// Copyright 2021 The Dawn & Tint Authors
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

#include "src/dawn/node/binding/GPUShaderModule.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/dawn/node/binding/Converter.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUShaderModule
////////////////////////////////////////////////////////////////////////////////
GPUShaderModule::GPUShaderModule(const wgpu::ShaderModuleDescriptor& desc,
                                 wgpu::ShaderModule shader,
                                 std::shared_ptr<AsyncRunner> async)
    : shader_(std::move(shader)), async_(std::move(async)), label_(CopyLabel(desc.label)) {}

interop::Promise<interop::Interface<interop::GPUCompilationInfo>>
GPUShaderModule::getCompilationInfo(Napi::Env env) {
    struct GPUCompilationMessage : public interop::GPUCompilationMessage {
        interop::GPUCompilationMessageType type;
        uint64_t lineNum;
        uint64_t linePos;
        uint64_t offset;
        uint64_t length;
        std::string message;

        explicit GPUCompilationMessage(const wgpu::CompilationMessage& m)
            : lineNum(m.lineNum),
              message(m.message) {
            bool foundUtf16 = false;
            for (const auto* chain = m.nextInChain; chain != nullptr; chain = chain->nextInChain) {
                if (chain->sType == wgpu::SType::DawnCompilationMessageUtf16) {
                    assert(!foundUtf16);
                    foundUtf16 = true;
                    const auto* utf16 =
                        reinterpret_cast<const wgpu::DawnCompilationMessageUtf16*>(chain);
                    linePos = utf16->linePos;
                    offset = utf16->offset;
                    length = utf16->length;
                }
            }
            assert(foundUtf16);

            switch (m.type) {
                case wgpu::CompilationMessageType::Error:
                    type = interop::GPUCompilationMessageType::kError;
                    break;
                case wgpu::CompilationMessageType::Warning:
                    type = interop::GPUCompilationMessageType::kWarning;
                    break;
                case wgpu::CompilationMessageType::Info:
                    type = interop::GPUCompilationMessageType::kInfo;
                    break;
                default:
                    UNREACHABLE("unrecognized handled compilation message type", m.type);
            }
        }

        std::string getMessage(Napi::Env) override { return message; }
        interop::GPUCompilationMessageType getType(Napi::Env) override { return type; }
        uint64_t getLineNum(Napi::Env) override { return lineNum; }
        uint64_t getLinePos(Napi::Env) override { return linePos; }
        uint64_t getOffset(Napi::Env) override { return offset; }
        uint64_t getLength(Napi::Env) override { return length; }
    };

    using Messages = std::vector<interop::Interface<interop::GPUCompilationMessage>>;

    struct GPUCompilationInfo : public interop::GPUCompilationInfo {
        std::vector<Napi::ObjectReference> messages;

        GPUCompilationInfo(Napi::Env env, Messages msgs) {
            messages.reserve(msgs.size());
            for (auto& msg : msgs) {
                messages.emplace_back(Napi::Persistent(Napi::Object(env, msg)));
            }
        }
        Messages getMessages(Napi::Env) override {
            Messages out;
            out.reserve(messages.size());
            for (auto& msg : messages) {
                out.emplace_back(msg.Value());
            }
            return out;
        }
    };

    auto ctx = std::make_unique<AsyncContext<interop::Interface<interop::GPUCompilationInfo>>>(
        env, PROMISE_INFO, async_);
    auto promise = ctx->promise;

    shader_.GetCompilationInfo(
        wgpu::CallbackMode::AllowProcessEvents,
        [ctx = std::move(ctx)](wgpu::CompilationInfoRequestStatus status,
                               wgpu::CompilationInfo const* compilationInfo) {
            Messages messages(compilationInfo->messageCount);
            for (uint32_t i = 0; i < compilationInfo->messageCount; i++) {
                auto& msg = compilationInfo->messages[i];
                messages[i] =
                    interop::GPUCompilationMessage::Create<GPUCompilationMessage>(ctx->env, msg);
            }

            ctx->promise.Resolve(interop::GPUCompilationInfo::Create<GPUCompilationInfo>(
                ctx->env, ctx->env, std::move(messages)));
        });

    return promise;
}

std::string GPUShaderModule::getLabel(Napi::Env) {
    return label_;
}

void GPUShaderModule::setLabel(Napi::Env, std::string value) {
    shader_.SetLabel(std::string_view(value));
    label_ = value;
}

}  // namespace wgpu::binding
