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

#include "src/dawn/node/binding/Errors.h"

namespace wgpu::binding {

namespace {
constexpr char kHierarchyRequestError[] = "HierarchyRequestError";
constexpr char kWrongDocumentError[] = "WrongDocumentError";
constexpr char kInvalidCharacterError[] = "InvalidCharacterError";
constexpr char kNoModificationAllowedError[] = "NoModificationAllowedError";
constexpr char kNotFoundError[] = "NotFoundError";
constexpr char kNotSupportedError[] = "NotSupportedError";
constexpr char kInUseAttributeError[] = "InUseAttributeError";
constexpr char kInvalidStateError[] = "InvalidStateError";
constexpr char kSyntaxError[] = "SyntaxError";
constexpr char kInvalidModificationError[] = "InvalidModificationError";
constexpr char kNamespaceError[] = "NamespaceError";
constexpr char kSecurityError[] = "SecurityError";
constexpr char kNetworkError[] = "NetworkError";
constexpr char kAbortError[] = "AbortError";
constexpr char kURLMismatchError[] = "URLMismatchError";
constexpr char kQuotaExceededError[] = "QuotaExceededError";
constexpr char kTimeoutError[] = "TimeoutError";
constexpr char kInvalidNodeTypeError[] = "InvalidNodeTypeError";
constexpr char kDataCloneError[] = "DataCloneError";
constexpr char kEncodingError[] = "EncodingError";
constexpr char kNotReadableError[] = "NotReadableError";
constexpr char kUnknownError[] = "UnknownError";
constexpr char kConstraintError[] = "ConstraintError";
constexpr char kDataError[] = "DataError";
constexpr char kTransactionInactiveError[] = "TransactionInactiveError";
constexpr char kReadOnlyError[] = "ReadOnlyError";
constexpr char kVersionError[] = "VersionError";
constexpr char kOperationError[] = "OperationError";
constexpr char kNotAllowedError[] = "NotAllowedError";
constexpr char kGPUPipelineError[] = "GPUPipelineError";

static Napi::Error New(Napi::Env env, std::string name, std::string message, uint16_t code = 0) {
    auto err = Napi::Error::New(env);
    err.Set("name", name);
    err.Set("message", message.empty() ? name : message);
    err.Set("code", static_cast<double>(code));
    return err;
}

}  // namespace

Napi::Error Errors::HierarchyRequestError(Napi::Env env, std::string message) {
    return New(env, kHierarchyRequestError, message);
}

Napi::Error Errors::WrongDocumentError(Napi::Env env, std::string message) {
    return New(env, kWrongDocumentError, message);
}

Napi::Error Errors::InvalidCharacterError(Napi::Env env, std::string message) {
    return New(env, kInvalidCharacterError, message);
}

Napi::Error Errors::NoModificationAllowedError(Napi::Env env, std::string message) {
    return New(env, kNoModificationAllowedError, message);
}

Napi::Error Errors::NotFoundError(Napi::Env env, std::string message) {
    return New(env, kNotFoundError, message);
}

Napi::Error Errors::NotSupportedError(Napi::Env env, std::string message) {
    return New(env, kNotSupportedError, message);
}

Napi::Error Errors::InUseAttributeError(Napi::Env env, std::string message) {
    return New(env, kInUseAttributeError, message);
}

Napi::Error Errors::InvalidStateError(Napi::Env env, std::string message) {
    return New(env, kInvalidStateError, message);
}

Napi::Error Errors::SyntaxError(Napi::Env env, std::string message) {
    return New(env, kSyntaxError, message);
}

Napi::Error Errors::InvalidModificationError(Napi::Env env, std::string message) {
    return New(env, kInvalidModificationError, message);
}

Napi::Error Errors::NamespaceError(Napi::Env env, std::string message) {
    return New(env, kNamespaceError, message);
}

Napi::Error Errors::SecurityError(Napi::Env env, std::string message) {
    return New(env, kSecurityError, message);
}

Napi::Error Errors::NetworkError(Napi::Env env, std::string message) {
    return New(env, kNetworkError, message);
}

Napi::Error Errors::AbortError(Napi::Env env, std::string message) {
    return New(env, kAbortError, message);
}

Napi::Error Errors::URLMismatchError(Napi::Env env, std::string message) {
    return New(env, kURLMismatchError, message);
}

Napi::Error Errors::QuotaExceededError(Napi::Env env, std::string message) {
    return New(env, kQuotaExceededError, message);
}

Napi::Error Errors::TimeoutError(Napi::Env env, std::string message) {
    return New(env, kTimeoutError, message);
}

Napi::Error Errors::InvalidNodeTypeError(Napi::Env env, std::string message) {
    return New(env, kInvalidNodeTypeError, message);
}

Napi::Error Errors::DataCloneError(Napi::Env env, std::string message) {
    return New(env, kDataCloneError, message);
}

Napi::Error Errors::EncodingError(Napi::Env env, std::string message) {
    return New(env, kEncodingError, message);
}

Napi::Error Errors::NotReadableError(Napi::Env env, std::string message) {
    return New(env, kNotReadableError, message);
}

Napi::Error Errors::UnknownError(Napi::Env env, std::string message) {
    return New(env, kUnknownError, message);
}

Napi::Error Errors::ConstraintError(Napi::Env env, std::string message) {
    return New(env, kConstraintError, message);
}

Napi::Error Errors::DataError(Napi::Env env, std::string message) {
    return New(env, kDataError, message);
}

Napi::Error Errors::TransactionInactiveError(Napi::Env env, std::string message) {
    return New(env, kTransactionInactiveError, message);
}

Napi::Error Errors::ReadOnlyError(Napi::Env env, std::string message) {
    return New(env, kReadOnlyError, message);
}

Napi::Error Errors::VersionError(Napi::Env env, std::string message) {
    return New(env, kVersionError, message);
}

Napi::Error Errors::OperationError(Napi::Env env, std::string message) {
    return New(env, kOperationError, message);
}

Napi::Error Errors::NotAllowedError(Napi::Env env, std::string message) {
    return New(env, kNotAllowedError, message);
}

Napi::Error Errors::GPUPipelineError(Napi::Env env, std::string message) {
    return New(env, kGPUPipelineError, message);
}

}  // namespace wgpu::binding
