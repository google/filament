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

{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
#include "{{native_dir}}/ChainUtils.h"

#include <tuple>
#include <utility>

namespace {{native_namespace}} {

{% set namespace = metadata.namespace %}
// Returns true iff the chain's SType matches the extension, false otherwise. If the SType was
// not already matched, sets the unpacked result accordingly. Otherwise, stores the duplicated
// SType in 'duplicate'.
template <typename Root, typename UnpackedPtrT, typename Ext>
bool UnpackExtension(typename UnpackedPtrT::TupleType& unpacked,
                     typename UnpackedPtrT::BitsetType& bitset,
                     typename UnpackedPtrT::ChainType chain, bool* duplicate) {
    DAWN_ASSERT(chain != nullptr);
    if (chain->sType == STypeFor<Ext>) {
        auto& member = std::get<Ext>(unpacked);
        if (member != nullptr && duplicate) {
            *duplicate = true;
        } else {
            member = reinterpret_cast<Ext>(chain);
            bitset.set(detail::UnpackedPtrIndexOf<UnpackedPtrT, Ext>);
        }
        return true;
    }
    return false;
}

// Tries to match all possible extensions, returning true iff one of the allowed extensions were
// matched, false otherwise. If the SType was not already matched, sets the unpacked result
// accordingly. Otherwise, stores the duplicated SType in 'duplicate'.
template <typename Root, typename UnpackedPtrT, typename AdditionalExts>
struct AdditionalExtensionUnpacker;
template <typename Root, typename UnpackedPtrT, typename... Exts>
struct AdditionalExtensionUnpacker<Root, UnpackedPtrT, detail::AdditionalExtensionsList<Exts...>> {
    static bool Unpack(typename UnpackedPtrT::TupleType& unpacked,
                       typename UnpackedPtrT::BitsetType& bitset,
                       typename UnpackedPtrT::ChainType chain,
                       bool* duplicate) {
        return ((UnpackExtension<Root, UnpackedPtrT, Exts>(unpacked, bitset, chain, duplicate)) ||
                ...);
    }
};

//
// UnpackedPtr chain helpers.
//
{% for type in by_category["structure"] %}
    {% if not type.extensible %}
        {% continue %}
    {% endif %}
    {% set T = as_cppType(type.name) %}
    {% set UnpackedPtrT = "UnpackedPtr<" + T + ">" %}
    template <>
    {{UnpackedPtrT}} Unpack<{{T}}>(typename {{UnpackedPtrT}}::PtrType chain) {
        {{UnpackedPtrT}} result(chain);
        for (typename {{UnpackedPtrT}}::ChainType next = chain->nextInChain;
             next != nullptr;
             next = next->nextInChain) {
            switch (next->sType) {
                {% for extension in type.extensions %}
                    {% set Ext = as_cppType(extension.name) %}
                    case STypeFor<{{Ext}}>: {
                        using ExtPtrType =
                            typename detail::PtrTypeFor<{{UnpackedPtrT}}, {{Ext}}>::Type;
                        std::get<ExtPtrType>(result.mUnpacked) =
                            static_cast<ExtPtrType>(next);
                        result.mBitset.set(
                            detail::UnpackedPtrIndexOf<{{UnpackedPtrT}}, ExtPtrType>
                        );
                        break;
                    }
                {% endfor %}
                default: {
                    using Unpacker =
                        AdditionalExtensionUnpacker<
                            {{T}},
                            {{UnpackedPtrT}},
                            detail::AdditionalExtensions<{{T}}>::List>;
                    Unpacker::Unpack(result.mUnpacked, result.mBitset, next, nullptr);
                    break;
                }
            }
        }
        return result;
    }
    template <>
    ResultOrError<{{UnpackedPtrT}}> ValidateAndUnpack<{{T}}>(
        typename {{UnpackedPtrT}}::PtrType chain) {
        {{UnpackedPtrT}} result(chain);
        for (typename {{UnpackedPtrT}}::ChainType next = chain->nextInChain;
             next != nullptr;
             next = next->nextInChain) {
            bool duplicate = false;
            switch (next->sType) {
                {% for extension in type.extensions %}
                    {% set Ext = as_cppType(extension.name) %}
                    case STypeFor<{{Ext}}>: {
                        using ExtPtrType =
                            typename detail::PtrTypeFor<{{UnpackedPtrT}}, {{Ext}}>::Type;
                        auto& member = std::get<ExtPtrType>(result.mUnpacked);
                        if (member != nullptr) {
                            duplicate = true;
                        } else {
                            member = static_cast<ExtPtrType>(next);
                            result.mBitset.set(
                                detail::UnpackedPtrIndexOf<{{UnpackedPtrT}}, ExtPtrType>
                            );
                        }
                        break;
                    }
                {% endfor %}
                default: {
                    using Unpacker =
                        AdditionalExtensionUnpacker<
                            {{T}},
                            {{UnpackedPtrT}},
                            detail::AdditionalExtensions<{{T}}>::List>;
                    if (!Unpacker::Unpack(result.mUnpacked,
                                          result.mBitset,
                                          next,
                                          &duplicate)) {
                        if (next->sType == wgpu::SType::DawnInjectedInvalidSType) {
                            // TODO(crbug.com/399470698): Need to reinterpret cast to base C type
                            // for now because in/out typing are differentiated in C++ bindings.
                            auto* ext = reinterpret_cast<const WGPUDawnInjectedInvalidSType*>(next);
                            return DAWN_VALIDATION_ERROR(
                                "Unexpected chained struct of type %s found on %s chain.",
                                wgpu::SType(ext->invalidSType), "{{T}}"
                            );
                        } else {
                            return DAWN_VALIDATION_ERROR(
                                "Unexpected chained struct of type %s found on %s chain.",
                                next->sType, "{{T}}"
                            );
                        }
                    }
                    break;
                }
            }
            if (duplicate) {
                return DAWN_VALIDATION_ERROR(
                    "Duplicate chained struct of type %s found on %s chain.",
                    next->sType, "{{T}}"
                );
            }
        }
        return result;
    }
{% endfor %}

}  // namespace {{native_namespace}}
