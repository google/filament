//* Copyright 2025 The Dawn & Tint Authors
//*
//* Redistribution and use in source and binary forms, with or without
//* modification, are permitted provided that the following conditions are met:
//*
//* 1. Redistributions of source code must retain the above copyright notice, this
//*    list of conditions and the following disclaimer.
//*
//* 2. Redistributions in binary form must reproduce the above copyright notice,
//*    this list of conditions and the following disclaimer in the documentation
//*    and/or other materials provided with the distribution.
//*
//* 3. Neither the name of the copyright holder nor the names of its
//*    contributors may be used to endorse or promote products derived from
//*    this software without specific prior written permission.
//*
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#ifndef SRC_DAWN_UTILS_COMBOLIMITS_H_
#define SRC_DAWN_UTILS_COMBOLIMITS_H_

#include <webgpu/webgpu_cpp.h>

#include <concepts>

#include "dawn/common/NonMovable.h"

namespace dawn::utils {

{% set limits_and_extensions = [types['limits']] + types['limits'].extensions %}
class ComboLimits : public NonMovable
    {% for type in limits_and_extensions %}
            , private wgpu::{{as_cppType(type.name)}}
    {% endfor %}
    {
  public:
    ComboLimits();

    // This is not copyable or movable to avoid surprises with nextInChain pointers becoming stale
    // (or getting replaced with nullptr). This explicit copy makes it clear what happens.
    void UnlinkedCopyTo(ComboLimits*) const;

    // Modify the ComboLimits in-place to link the extension structs correctly, and return the base
    // struct. Optionally accepts any number of additional structs to add to the
    // end of the chain, e.g.: `comboLimits.GetLinked(&extension1, &extension2)`.
    // Always use GetLinked (rather than `&comboLimits`) whenever passing a ComboLimits to the API.
    template <typename... Extension>
        requires (std::convertible_to<Extension, wgpu::ChainedStructOut*> && ...)
    wgpu::Limits* GetLinked(Extension... extension) {
        wgpu::ChainedStructOut* lastExtension = nullptr;
        // Link all of the standard extensions.
        {% for type in types['limits'].extensions %}
            {% if loop.first %}
                lastExtension = this->wgpu::Limits::nextInChain =
            {% else %}
                lastExtension = lastExtension->nextInChain =
            {% endif %}
                static_cast<wgpu::{{as_cppType(type.name)}}*>(this);
        {% endfor %}
        // Link any extensions passed by the caller.
        ((lastExtension = lastExtension->nextInChain = extension), ...);
        lastExtension->nextInChain = nullptr;
        return this;
    }

    {% for type in limits_and_extensions %}
        {% for member in type.members %}
            using wgpu::{{as_cppType(type.name)}}::{{as_varName(member.name)}};
        {% endfor %}
    {% endfor %}
};

}  // namespace dawn::utils

#endif  // SRC_DAWN_UTILS_COMBOLIMITS_H_
