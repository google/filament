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

#ifndef SRC_DAWN_NODE_BINDING_ERRORS_H_
#define SRC_DAWN_NODE_BINDING_ERRORS_H_

#include <string>
#include <tuple>

#include "src/dawn/node/interop/Core.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// Errors contains static helper methods for creating DOMException error
// messages as documented at:
// https://heycam.github.io/webidl/#idl-DOMException-error-names
class Errors {
  public:
    static Napi::Error HierarchyRequestError(Napi::Env, std::string message = {});
    static Napi::Error WrongDocumentError(Napi::Env, std::string message = {});
    static Napi::Error InvalidCharacterError(Napi::Env, std::string message = {});
    static Napi::Error NoModificationAllowedError(Napi::Env, std::string message = {});
    static Napi::Error NotFoundError(Napi::Env, std::string message = {});
    static Napi::Error NotSupportedError(Napi::Env, std::string message = {});
    static Napi::Error InUseAttributeError(Napi::Env, std::string message = {});
    static Napi::Error InvalidStateError(Napi::Env, std::string message = {});
    static Napi::Error SyntaxError(Napi::Env, std::string message = {});
    static Napi::Error InvalidModificationError(Napi::Env, std::string message = {});
    static Napi::Error NamespaceError(Napi::Env, std::string message = {});
    static Napi::Error SecurityError(Napi::Env, std::string message = {});
    static Napi::Error NetworkError(Napi::Env, std::string message = {});
    static Napi::Error AbortError(Napi::Env, std::string message = {});
    static Napi::Error URLMismatchError(Napi::Env, std::string message = {});
    static Napi::Error QuotaExceededError(Napi::Env, std::string message = {});
    static Napi::Error TimeoutError(Napi::Env, std::string message = {});
    static Napi::Error InvalidNodeTypeError(Napi::Env, std::string message = {});
    static Napi::Error DataCloneError(Napi::Env, std::string message = {});
    static Napi::Error EncodingError(Napi::Env, std::string message = {});
    static Napi::Error NotReadableError(Napi::Env, std::string message = {});
    static Napi::Error UnknownError(Napi::Env, std::string message = {});
    static Napi::Error ConstraintError(Napi::Env, std::string message = {});
    static Napi::Error DataError(Napi::Env, std::string message = {});
    static Napi::Error TransactionInactiveError(Napi::Env, std::string message = {});
    static Napi::Error ReadOnlyError(Napi::Env, std::string message = {});
    static Napi::Error VersionError(Napi::Env, std::string message = {});
    static Napi::Error OperationError(Napi::Env, std::string message = {});
    static Napi::Error NotAllowedError(Napi::Env, std::string message = {});
};

class GPUOutOfMemoryError : public interop::GPUOutOfMemoryError {
  public:
    explicit GPUOutOfMemoryError(const std::tuple<std::string>& args);

    std::string getMessage(Napi::Env) override;

  private:
    std::string message_;
};

class GPUValidationError : public interop::GPUValidationError {
  public:
    explicit GPUValidationError(const std::tuple<std::string>& args);

    std::string getMessage(Napi::Env) override;

  private:
    std::string message_;
};

class GPUInternalError : public interop::GPUInternalError {
  public:
    explicit GPUInternalError(const std::tuple<std::string>& args);

    std::string getMessage(Napi::Env) override;

  private:
    std::string message_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_ERRORS_H_
