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

#include "dawn/native/webgpu_absl_format.h"

#include <string>
#include <vector>

#include "dawn/common/MatchVariant.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/AttachmentState.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/Device.h"
#include "dawn/native/Format.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/ProgrammableEncoder.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/ShaderModule.h"
#include "dawn/native/Subresource.h"
#include "dawn/native/Surface.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

//
// Structs
//

absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
AbslFormatConvert(const Color* value, const absl::FormatConversionSpec& spec, absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(
        absl::StrFormat("[Color r:%f, g:%f, b:%f, a:%f]", value->r, value->g, value->b, value->a));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Extent2D* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(absl::StrFormat("[Extent2D width:%u, height:%u]", value->width, value->height));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Extent3D* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(absl::StrFormat("[Extent3D width:%u, height:%u, depthOrArrayLayers:%u]", value->width,
                              value->height, value->depthOrArrayLayers));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Origin2D* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(absl::StrFormat("[Origin2D x:%u, y:%u]", value->x, value->y));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Origin3D* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(absl::StrFormat("[Origin3D x:%u, y:%u, z:%u]", value->x, value->y, value->z));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const BindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    s->Append(absl::StrFormat("{ binding: %u, visibility: %s, ", value.binding, value.visibility));
    if (value.arraySize != BindingIndex(1)) {
        s->Append(absl::StrFormat("arraySize: %u, indexInArray: %u, ", value.arraySize,
                                  value.indexInArray));
    }

    MatchVariant(
        value.bindingLayout,
        [&](const BufferBindingInfo& layout) {
            s->Append(absl::StrFormat("%s: %s ", BindingInfoType::Buffer, layout));
        },
        [&](const SamplerBindingInfo& layout) {
            s->Append(absl::StrFormat("%s: %s ", BindingInfoType::Sampler, layout));
        },
        [&](const StaticSamplerBindingInfo& layout) {
            s->Append(absl::StrFormat("%s: %s ", BindingInfoType::StaticSampler, layout));
        },
        [&](const TextureBindingInfo& layout) {
            s->Append(absl::StrFormat("%s: %s ", BindingInfoType::Texture, layout));
        },
        [&](const StorageTextureBindingInfo& layout) {
            s->Append(absl::StrFormat("%s: %s ", BindingInfoType::StorageTexture, layout));
        },
        [&](const InputAttachmentBindingInfo& layout) {
            s->Append(absl::StrFormat("%s: %s ", BindingInfoType::InputAttachment, layout));
        });

    s->Append(absl::StrFormat("}"));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const BufferBindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    s->Append(absl::StrFormat("{type: %s, minBindingSize: %u, hasDynamicOffset: %u}", value.type,
                              value.minBindingSize, value.hasDynamicOffset));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const BufferBindingLayout& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    auto info = BufferBindingInfo::From(value);
    return AbslFormatConvert(info, spec, s);
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const TextureBindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    s->Append(absl::StrFormat("{sampleType: %s, viewDimension: %s, multisampled: %u}",
                              value.sampleType, value.viewDimension, value.multisampled));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const TextureBindingLayout& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    auto info = TextureBindingInfo::From(value);
    return AbslFormatConvert(info, spec, s);
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const StorageTextureBindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    s->Append(absl::StrFormat("{format: %s, viewDimension: %s, access: %s}", value.format,
                              value.viewDimension, value.access));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const StorageTextureBindingLayout& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    auto info = StorageTextureBindingInfo::From(value);
    return AbslFormatConvert(info, spec, s);
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const SamplerBindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    s->Append(absl::StrFormat("{type: %s}", value.type));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const SamplerBindingLayout& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    auto info = SamplerBindingInfo::From(value);
    return AbslFormatConvert(info, spec, s);
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const StaticSamplerBindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    s->Append(absl::StrFormat("{sampler: %s}", value.sampler.Get()));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const InputAttachmentBindingInfo& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    s->Append(absl::StrFormat("{sampleType: %s}", value.sampleType));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const TexelCopyTextureInfo* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(
        absl::StrFormat("[TexelCopyTextureInfo texture: %s, mipLevel: %u, origin: %s, aspect: %s]",
                        value->texture, value->mipLevel, &value->origin, value->aspect));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const TexelCopyBufferLayout* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(absl::StrFormat("[TexelCopyBufferLayout offset:%u, bytesPerRow:%u, rowsPerImage:%u]",
                              value->offset, value->bytesPerRow, value->rowsPerImage));
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const ShaderModuleEntryPoint* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append(absl::StrFormat("[EntryPoint \"%s\"", value->name));
    if (value->defaulted) {
        s->Append(" (defaulted)");
    }
    s->Append("]");
    return {true};
}

//
// Objects
//

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const AdapterBase* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append("[Adapter");
    const std::string& name = value->GetName();
    if (!name.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", name));
    }
    s->Append("]");
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const DeviceBase* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append("[Device");
    const std::string& label = value->GetLabel();
    if (!label.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", label));
    }
    s->Append("]");
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const ApiObjectBase* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append("[");
    if (value->IsError()) {
        s->Append("Invalid ");
    }
    value->FormatLabel(s);
    s->Append("]");
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const AttachmentState* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }

    s->Append("{ colorTargets: [");

    ColorAttachmentIndex nextColorIndex{};

    bool needsComma = false;
    for (auto i : value->GetColorAttachmentsMask()) {
        if (needsComma) {
            s->Append(", ");
        }

        while (nextColorIndex < i) {
            s->Append(absl::StrFormat("%d={format: %s}, ", nextColorIndex,
                                      wgpu::TextureFormat::Undefined));
            nextColorIndex++;
            needsComma = false;
        }

        s->Append(absl::StrFormat("%d={format:%s", i, value->GetColorAttachmentFormat(i)));

        if (value->GetExpandResolveInfo().attachmentsToExpandResolve.any()) {
            s->Append(
                absl::StrFormat(", resolve:%v, expandResolve:%v",
                                value->GetExpandResolveInfo().resolveTargetsMask.test(i),
                                value->GetExpandResolveInfo().attachmentsToExpandResolve.test(i)));
        }
        s->Append("}");

        nextColorIndex++;
        needsComma = true;
    }

    s->Append("], ");

    if (value->HasDepthStencilAttachment()) {
        s->Append(absl::StrFormat("depthStencilFormat: %s, ", value->GetDepthStencilFormat()));
    }

    s->Append(absl::StrFormat("sampleCount: %u", value->GetSampleCount()));

    if (value->HasPixelLocalStorage()) {
        const std::vector<wgpu::TextureFormat>& plsSlots = value->GetStorageAttachmentSlots();
        s->Append(absl::StrFormat(", totalPixelLocalStorageSize: %d",
                                  plsSlots.size() * kPLSSlotByteSize));
        s->Append(", storageAttachments: [ ");
        for (size_t i = 0; i < plsSlots.size(); i++) {
            if (plsSlots[i] != wgpu::TextureFormat::Undefined) {
                s->Append(absl::StrFormat("{format: %s, offset: %d}, ", plsSlots[i],
                                          i * kPLSSlotByteSize));
            }
        }
        s->Append("]");
    }

    s->Append(" }");

    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const Surface* value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == nullptr) {
        s->Append("[null]");
        return {true};
    }
    s->Append("[Surface");
    const std::string& label = value->GetLabel();
    if (!label.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", label));
    }
    s->Append("]");
    return {true};
}

//
// Enums
//

absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
AbslFormatConvert(Aspect value, const absl::FormatConversionSpec& spec, absl::FormatSink* s) {
    if (value == Aspect::None) {
        s->Append("None");
        return {true};
    }

    bool first = true;

    if (value & Aspect::Color) {
        first = false;
        s->Append("Color");
        value &= ~Aspect::Color;
    }

    if (value & Aspect::Depth) {
        if (!first) {
            s->Append("|");
        }
        first = false;
        s->Append("Depth");
        value &= ~Aspect::Depth;
    }

    if (value & Aspect::Stencil) {
        if (!first) {
            s->Append("|");
        }
        first = false;
        s->Append("Stencil");
        value &= ~Aspect::Stencil;
    }

    // Output any remaining flags as a hex value
    if (static_cast<bool>(value)) {
        if (!first) {
            s->Append("|");
        }
        s->Append(absl::StrFormat("%x", static_cast<uint8_t>(value)));
    }

    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    SampleTypeBit value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    if (value == SampleTypeBit::None) {
        s->Append("None");
        return {true};
    }

    bool first = true;

    if (value & SampleTypeBit::Float) {
        first = false;
        s->Append("Float");
        value &= ~SampleTypeBit::Float;
    }

    if (value & SampleTypeBit::UnfilterableFloat) {
        if (!first) {
            s->Append("|");
        }
        first = false;
        s->Append("UnfilterableFloat");
        value &= ~SampleTypeBit::UnfilterableFloat;
    }

    if (value & SampleTypeBit::Depth) {
        if (!first) {
            s->Append("|");
        }
        first = false;
        s->Append("Depth");
        value &= ~SampleTypeBit::Depth;
    }

    if (value & SampleTypeBit::Sint) {
        if (!first) {
            s->Append("|");
        }
        first = false;
        s->Append("Sint");
        value &= ~SampleTypeBit::Sint;
    }

    if (value & SampleTypeBit::Uint) {
        if (!first) {
            s->Append("|");
        }
        first = false;
        s->Append("Uint");
        value &= ~SampleTypeBit::Uint;
    }

    // Output any remaining flags as a hex value
    if (static_cast<bool>(value)) {
        if (!first) {
            s->Append("|");
        }
        s->Append(absl::StrFormat("%x", static_cast<uint8_t>(value)));
    }

    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    BindingInfoType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case BindingInfoType::Buffer:
            s->Append("buffer");
            break;
        case BindingInfoType::Sampler:
            s->Append("sampler");
            break;
        case BindingInfoType::Texture:
            s->Append("texture");
            break;
        case BindingInfoType::StorageTexture:
            s->Append("storageTexture");
            break;
        case BindingInfoType::ExternalTexture:
            s->Append("externalTexture");
            break;
        case BindingInfoType::StaticSampler:
            s->Append("staticSampler");
            break;
        case BindingInfoType::InputAttachment:
            s->Append("inputAttachment");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    SingleShaderStage value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case SingleShaderStage::Compute:
            s->Append("Compute");
            break;
        case SingleShaderStage::Vertex:
            s->Append("Vertex");
            break;
        case SingleShaderStage::Fragment:
            s->Append("Fragment");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    VertexFormatBaseType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case VertexFormatBaseType::Float:
            s->Append("Float");
            break;
        case VertexFormatBaseType::Uint:
            s->Append("Uint");
            break;
        case VertexFormatBaseType::Sint:
            s->Append("Sint");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    InterStageComponentType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case InterStageComponentType::F32:
            s->Append("f32");
            break;
        case InterStageComponentType::F16:
            s->Append("f16");
            break;
        case InterStageComponentType::U32:
            s->Append("u32");
            break;
        case InterStageComponentType::I32:
            s->Append("i32");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    InterpolationType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case InterpolationType::Perspective:
            s->Append("Perspective");
            break;
        case InterpolationType::Linear:
            s->Append("Linear");
            break;
        case InterpolationType::Flat:
            s->Append("Flat");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    InterpolationSampling value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case InterpolationSampling::None:
            s->Append("None");
            break;
        case InterpolationSampling::Center:
            s->Append("Center");
            break;
        case InterpolationSampling::Centroid:
            s->Append("Centroid");
            break;
        case InterpolationSampling::Sample:
            s->Append("Sample");
            break;
        case InterpolationSampling::First:
            s->Append("First");
            break;
        case InterpolationSampling::Either:
            s->Append("Either");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    TextureComponentType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case TextureComponentType::Float:
            s->Append("Float");
            break;
        case TextureComponentType::Sint:
            s->Append("Sint");
            break;
        case TextureComponentType::Uint:
            s->Append("Uint");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    PixelLocalMemberType value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case PixelLocalMemberType::I32:
            s->Append("i32");
            break;
        case PixelLocalMemberType::U32:
            s->Append("u32");
            break;
        case PixelLocalMemberType::F32:
            s->Append("f32");
            break;
    }
    return {true};
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
AbslFormatConvert(StringView value, const absl::FormatConversionSpec& spec, absl::FormatSink* s) {
    if (value.IsUndefined()) {
        s->Append("[undefined]");
        return {true};
    }

    s->Append("\"");
    s->Append(absl::string_view(value));
    s->Append("\"");
    return {true};
}

}  // namespace dawn::native
