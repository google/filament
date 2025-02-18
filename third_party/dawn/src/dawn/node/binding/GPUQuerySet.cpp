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

#include "src/dawn/node/binding/GPUQuerySet.h"

#include <utility>

#include "src/dawn/node/binding/Converter.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUQuerySet
////////////////////////////////////////////////////////////////////////////////
GPUQuerySet::GPUQuerySet(const wgpu::QuerySetDescriptor& desc, wgpu::QuerySet query_set)
    : query_set_(std::move(query_set)), label_(CopyLabel(desc.label)) {}

void GPUQuerySet::destroy(Napi::Env) {
    query_set_.Destroy();
}

interop::GPUQueryType GPUQuerySet::getType(Napi::Env env) {
    interop::GPUQueryType result;

    Converter conv(env);
    if (!conv(result, query_set_.GetType())) {
        Napi::Error::New(env, "Couldn't convert type to a JavaScript value.")
            .ThrowAsJavaScriptException();
        return interop::GPUQueryType::kOcclusion;  // Doesn't get used.
    }

    return result;
}

interop::GPUSize32Out GPUQuerySet::getCount(Napi::Env) {
    return query_set_.GetCount();
}

std::string GPUQuerySet::getLabel(Napi::Env) {
    return label_;
}

void GPUQuerySet::setLabel(Napi::Env, std::string value) {
    query_set_.SetLabel(std::string_view(value));
    label_ = value;
}

}  // namespace wgpu::binding
