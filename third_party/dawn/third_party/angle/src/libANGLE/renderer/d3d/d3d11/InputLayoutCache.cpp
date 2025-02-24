//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// InputLayoutCache.cpp: Defines InputLayoutCache, a class that builds and caches
// D3D11 input layouts.

#include "libANGLE/renderer/d3d/d3d11/InputLayoutCache.h"

#include "common/bitset_utils.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/Program.h"
#include "libANGLE/ProgramExecutable.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/renderer/d3d/IndexDataManager.h"
#include "libANGLE/renderer/d3d/ProgramD3D.h"
#include "libANGLE/renderer/d3d/VertexDataManager.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/ShaderExecutable11.h"
#include "libANGLE/renderer/d3d/d3d11/VertexArray11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"

namespace rx
{

namespace
{

GLenum GetGLSLAttributeType(const std::vector<gl::ProgramInput> &shaderAttributes, size_t index)
{
    // Count matrices differently
    for (const gl::ProgramInput &attrib : shaderAttributes)
    {
        if (attrib.getLocation() == -1)
        {
            continue;
        }

        GLenum transposedType = gl::TransposeMatrixType(attrib.getType());
        int rows              = gl::VariableRowCount(transposedType);
        int intIndex          = static_cast<int>(index);

        if (intIndex >= attrib.getLocation() && intIndex < attrib.getLocation() + rows)
        {
            return transposedType;
        }
    }

    UNREACHABLE();
    return GL_NONE;
}

struct PackedAttribute
{
    uint8_t attribType;
    uint8_t semanticIndex;
    uint8_t vertexFormatType;
    uint8_t unusedPadding;
    uint32_t divisor;
};

}  // anonymous namespace

PackedAttributeLayout::PackedAttributeLayout() : numAttributes(0), attributeData({}) {}

PackedAttributeLayout::PackedAttributeLayout(const PackedAttributeLayout &other) = default;

void PackedAttributeLayout::addAttributeData(GLenum glType,
                                             UINT semanticIndex,
                                             angle::FormatID vertexFormatID,
                                             unsigned int divisor)
{
    gl::AttributeType attribType = gl::GetAttributeType(glType);

    PackedAttribute packedAttrib;
    packedAttrib.attribType       = static_cast<uint8_t>(attribType);
    packedAttrib.semanticIndex    = static_cast<uint8_t>(semanticIndex);
    packedAttrib.vertexFormatType = static_cast<uint8_t>(vertexFormatID);
    packedAttrib.unusedPadding    = 0u;
    packedAttrib.divisor          = static_cast<uint32_t>(divisor);

    ASSERT(static_cast<gl::AttributeType>(packedAttrib.attribType) == attribType);
    ASSERT(static_cast<UINT>(packedAttrib.semanticIndex) == semanticIndex);
    ASSERT(static_cast<angle::FormatID>(packedAttrib.vertexFormatType) == vertexFormatID);
    ASSERT(static_cast<unsigned int>(packedAttrib.divisor) == divisor);

    static_assert(sizeof(uint64_t) == sizeof(PackedAttribute),
                  "PackedAttributes must be 64-bits exactly.");

    attributeData[numAttributes++] = gl::bitCast<uint64_t>(packedAttrib);
}

bool PackedAttributeLayout::operator==(const PackedAttributeLayout &other) const
{
    return (numAttributes == other.numAttributes) && (attributeData == other.attributeData);
}

InputLayoutCache::InputLayoutCache() : mLayoutCache(kDefaultCacheSize * 2) {}

InputLayoutCache::~InputLayoutCache() {}

void InputLayoutCache::clear()
{
    mLayoutCache.Clear();
}

angle::Result InputLayoutCache::getInputLayout(
    Context11 *context11,
    const gl::State &state,
    const std::vector<const TranslatedAttribute *> &currentAttributes,
    const AttribIndexArray &sortedSemanticIndices,
    gl::PrimitiveMode mode,
    GLsizei vertexCount,
    GLsizei instances,
    const d3d11::InputLayout **inputLayoutOut)
{
    gl::ProgramExecutable *executable = state.getProgramExecutable();
    const auto &shaderAttributes      = executable->getProgramInputs();
    PackedAttributeLayout layout;

    ProgramExecutableD3D *executableD3D = GetImplAs<ProgramExecutableD3D>(executable);
    const auto &attribs            = state.getVertexArray()->getVertexAttributes();
    const auto &bindings           = state.getVertexArray()->getVertexBindings();
    const auto &locationToSemantic = executableD3D->getAttribLocationToD3DSemantics();
    int divisorMultiplier          = executable->usesMultiview() ? executable->getNumViews() : 1;

    for (size_t attribIndex : executable->getActiveAttribLocationsMask())
    {
        // Record the type of the associated vertex shader vector in our key
        // This will prevent mismatched vertex shaders from using the same input layout
        GLenum glslElementType = GetGLSLAttributeType(shaderAttributes, attribIndex);

        const auto &attrib  = attribs[attribIndex];
        const auto &binding = bindings[attrib.bindingIndex];
        int d3dSemantic     = locationToSemantic[attribIndex];

        const auto &currentValue =
            state.getVertexAttribCurrentValue(static_cast<unsigned int>(attribIndex));
        angle::FormatID vertexFormatID = gl::GetVertexFormatID(attrib, currentValue.Type);

        layout.addAttributeData(glslElementType, d3dSemantic, vertexFormatID,
                                binding.getDivisor() * divisorMultiplier);
    }

    if (layout.numAttributes > 0)
    {
        auto it = mLayoutCache.Get(layout);
        if (it != mLayoutCache.end())
        {
            *inputLayoutOut = &it->second;
        }
        else
        {
            angle::TrimCache(mLayoutCache.max_size() / 2, kGCLimit, "input layout", &mLayoutCache);

            d3d11::InputLayout newInputLayout;
            ANGLE_TRY(createInputLayout(context11, sortedSemanticIndices, currentAttributes, mode,
                                        vertexCount, instances, &newInputLayout));

            auto insertIt   = mLayoutCache.Put(layout, std::move(newInputLayout));
            *inputLayoutOut = &insertIt->second;
        }
    }

    return angle::Result::Continue;
}

angle::Result InputLayoutCache::createInputLayout(
    Context11 *context11,
    const AttribIndexArray &sortedSemanticIndices,
    const std::vector<const TranslatedAttribute *> &currentAttributes,
    gl::PrimitiveMode mode,
    GLsizei vertexCount,
    GLsizei instances,
    d3d11::InputLayout *inputLayoutOut)
{
    Renderer11 *renderer                = context11->getRenderer();
    ProgramExecutableD3D *executableD3D = renderer->getStateManager()->getProgramExecutableD3D();
    D3D_FEATURE_LEVEL featureLevel      = renderer->getRenderer11DeviceCaps().featureLevel;

    unsigned int inputElementCount = 0;
    gl::AttribArray<D3D11_INPUT_ELEMENT_DESC> inputElements;

    for (size_t attribIndex = 0; attribIndex < currentAttributes.size(); ++attribIndex)
    {
        const auto &attrib    = *currentAttributes[attribIndex];
        const int sortedIndex = sortedSemanticIndices[attribIndex];

        D3D11_INPUT_CLASSIFICATION inputClass =
            attrib.divisor > 0 ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;

        angle::FormatID vertexFormatID =
            gl::GetVertexFormatID(*attrib.attribute, attrib.currentValueType);
        const auto &vertexFormatInfo = d3d11::GetVertexFormatInfo(vertexFormatID, featureLevel);

        auto *inputElement = &inputElements[inputElementCount];

        inputElement->SemanticName         = "TEXCOORD";
        inputElement->SemanticIndex        = sortedIndex;
        inputElement->Format               = vertexFormatInfo.nativeFormat;
        inputElement->InputSlot            = static_cast<UINT>(attribIndex);
        inputElement->AlignedByteOffset    = 0;
        inputElement->InputSlotClass       = inputClass;
        inputElement->InstanceDataStepRate = attrib.divisor;

        inputElementCount++;
    }

    ShaderExecutableD3D *shader = nullptr;
    ANGLE_TRY(executableD3D->getVertexExecutableForCachedInputLayout(context11, renderer, &shader,
                                                                     nullptr));

    ShaderExecutableD3D *shader11 = GetAs<ShaderExecutable11>(shader);

    InputElementArray inputElementArray(inputElements.data(), inputElementCount);
    ShaderData vertexShaderData(shader11->getFunction(), shader11->getLength());

    ANGLE_TRY(renderer->allocateResource(context11, inputElementArray, &vertexShaderData,
                                         inputLayoutOut));
    return angle::Result::Continue;
}

void InputLayoutCache::setCacheSize(size_t newCacheSize)
{
    // Forces a reset of the cache.
    LayoutCache newCache(newCacheSize);
    mLayoutCache.Swap(newCache);
}

}  // namespace rx
