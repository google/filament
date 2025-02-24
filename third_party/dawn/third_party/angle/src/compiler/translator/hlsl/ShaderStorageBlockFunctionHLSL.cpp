//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderStorageBlockFunctionHLSL: Wrapper functions for RWByteAddressBuffer Load/Store functions.
//

#include "compiler/translator/hlsl/ShaderStorageBlockFunctionHLSL.h"

#include "common/utilities.h"
#include "compiler/translator/blocklayout.h"
#include "compiler/translator/hlsl/UtilsHLSL.h"
#include "compiler/translator/hlsl/blocklayoutHLSL.h"
#include "compiler/translator/util.h"

namespace sh
{

// static
void ShaderStorageBlockFunctionHLSL::OutputSSBOLoadFunctionBody(
    TInfoSinkBase &out,
    const ShaderStorageBlockFunction &ssboFunction)
{
    const char *convertString;
    switch (ssboFunction.type.getBasicType())
    {
        case EbtFloat:
            convertString = "asfloat(";
            break;
        case EbtInt:
            convertString = "asint(";
            break;
        case EbtUInt:
            convertString = "asuint(";
            break;
        case EbtBool:
            convertString = "asint(";
            break;
        default:
            UNREACHABLE();
            return;
    }

    size_t bytesPerComponent =
        gl::VariableComponentSize(gl::VariableComponentType(GLVariableType(ssboFunction.type)));
    out << "    " << ssboFunction.typeString << " result";
    if (ssboFunction.type.isScalar())
    {
        size_t offset = ssboFunction.swizzleOffsets[0] * bytesPerComponent;
        out << " = " << convertString << "buffer.Load(loc + " << offset << "));\n ";
    }
    else if (ssboFunction.type.isVector())
    {
        if (ssboFunction.rowMajor || !ssboFunction.isDefaultSwizzle)
        {
            size_t componentStride = bytesPerComponent;
            if (ssboFunction.rowMajor)
            {
                componentStride = ssboFunction.matrixStride;
            }

            out << " = {";
            for (const int offset : ssboFunction.swizzleOffsets)
            {
                size_t offsetInBytes = offset * componentStride;
                out << convertString << "buffer.Load(loc + " << offsetInBytes << ")),";
            }
            out << "};\n";
        }
        else
        {
            out << " = " << convertString << "buffer.Load"
                << static_cast<uint32_t>(ssboFunction.type.getNominalSize()) << "(loc));\n";
        }
    }
    else if (ssboFunction.type.isMatrix())
    {
        if (ssboFunction.rowMajor)
        {
            out << ";";
            out << "    float" << static_cast<uint32_t>(ssboFunction.type.getRows()) << "x"
                << static_cast<uint32_t>(ssboFunction.type.getCols()) << " tmp_ = {";
            for (uint8_t rowIndex = 0; rowIndex < ssboFunction.type.getRows(); rowIndex++)
            {
                out << "asfloat(buffer.Load" << static_cast<uint32_t>(ssboFunction.type.getCols())
                    << "(loc + " << rowIndex * ssboFunction.matrixStride << ")), ";
            }
            out << "};\n";
            out << "    result = transpose(tmp_);\n";
        }
        else
        {
            out << " = {";
            for (uint8_t columnIndex = 0; columnIndex < ssboFunction.type.getCols(); columnIndex++)
            {
                out << "asfloat(buffer.Load" << static_cast<uint32_t>(ssboFunction.type.getRows())
                    << "(loc + " << columnIndex * ssboFunction.matrixStride << ")), ";
            }
            out << "};\n";
        }
    }
    else
    {
        // TODO(jiajia.qin@intel.com): Process all possible return types.
        // http://anglebug.com/40644618
        out << ";\n";
    }

    out << "    return result;\n";
    return;
}

// static
void ShaderStorageBlockFunctionHLSL::OutputSSBOStoreFunctionBody(
    TInfoSinkBase &out,
    const ShaderStorageBlockFunction &ssboFunction)
{
    size_t bytesPerComponent =
        gl::VariableComponentSize(gl::VariableComponentType(GLVariableType(ssboFunction.type)));
    if (ssboFunction.type.isScalar())
    {
        size_t offset = ssboFunction.swizzleOffsets[0] * bytesPerComponent;
        if (ssboFunction.type.getBasicType() == EbtBool)
        {
            out << "    buffer.Store(loc + " << offset << ", uint(value));\n";
        }
        else
        {
            out << "    buffer.Store(loc + " << offset << ", asuint(value));\n";
        }
    }
    else if (ssboFunction.type.isVector())
    {
        out << "    uint" << static_cast<uint32_t>(ssboFunction.type.getNominalSize())
            << " _value;\n";
        if (ssboFunction.type.getBasicType() == EbtBool)
        {
            out << "    _value = uint" << static_cast<uint32_t>(ssboFunction.type.getNominalSize())
                << "(value);\n";
        }
        else
        {
            out << "    _value = asuint(value);\n";
        }

        if (ssboFunction.rowMajor || !ssboFunction.isDefaultSwizzle)
        {
            size_t componentStride = bytesPerComponent;
            if (ssboFunction.rowMajor)
            {
                componentStride = ssboFunction.matrixStride;
            }
            const TVector<int> &swizzleOffsets = ssboFunction.swizzleOffsets;
            for (int index = 0; index < static_cast<int>(swizzleOffsets.size()); index++)
            {
                size_t offsetInBytes = swizzleOffsets[index] * componentStride;
                out << "buffer.Store(loc + " << offsetInBytes << ", _value[" << index << "]);\n";
            }
        }
        else
        {
            out << "    buffer.Store" << static_cast<uint32_t>(ssboFunction.type.getNominalSize())
                << "(loc, _value);\n";
        }
    }
    else if (ssboFunction.type.isMatrix())
    {
        if (ssboFunction.rowMajor)
        {
            out << "    float" << static_cast<uint32_t>(ssboFunction.type.getRows()) << "x"
                << static_cast<uint32_t>(ssboFunction.type.getCols())
                << " tmp_ = transpose(value);\n";
            for (uint8_t rowIndex = 0; rowIndex < ssboFunction.type.getRows(); rowIndex++)
            {
                out << "    buffer.Store" << static_cast<uint32_t>(ssboFunction.type.getCols())
                    << "(loc + " << rowIndex * ssboFunction.matrixStride << ", asuint(tmp_["
                    << static_cast<uint32_t>(rowIndex) << "]));\n";
            }
        }
        else
        {
            for (uint8_t columnIndex = 0; columnIndex < ssboFunction.type.getCols(); columnIndex++)
            {
                out << "    buffer.Store" << static_cast<uint32_t>(ssboFunction.type.getRows())
                    << "(loc + " << columnIndex * ssboFunction.matrixStride << ", asuint(value["
                    << static_cast<uint32_t>(columnIndex) << "]));\n";
            }
        }
    }
    else
    {
        // TODO(jiajia.qin@intel.com): Process all possible return types.
        // http://anglebug.com/40644618
    }
}

// static
void ShaderStorageBlockFunctionHLSL::OutputSSBOLengthFunctionBody(TInfoSinkBase &out,
                                                                  int unsizedArrayStride)
{
    out << "    uint dim = 0;\n";
    out << "    buffer.GetDimensions(dim);\n";
    out << "    return int((dim - loc)/uint(" << unsizedArrayStride << "));\n";
}

// static
void ShaderStorageBlockFunctionHLSL::OutputSSBOAtomicMemoryFunctionBody(
    TInfoSinkBase &out,
    const ShaderStorageBlockFunction &ssboFunction)
{
    out << "    " << ssboFunction.typeString << " original_value;\n";
    switch (ssboFunction.method)
    {
        case SSBOMethod::ATOMIC_ADD:
            out << "    buffer.InterlockedAdd(loc, value, original_value);\n";
            break;
        case SSBOMethod::ATOMIC_MIN:
            out << "    buffer.InterlockedMin(loc, value, original_value);\n";
            break;
        case SSBOMethod::ATOMIC_MAX:
            out << "    buffer.InterlockedMax(loc, value, original_value);\n";
            break;
        case SSBOMethod::ATOMIC_AND:
            out << "    buffer.InterlockedAnd(loc, value, original_value);\n";
            break;
        case SSBOMethod::ATOMIC_OR:
            out << "    buffer.InterlockedOr(loc, value, original_value);\n";
            break;
        case SSBOMethod::ATOMIC_XOR:
            out << "    buffer.InterlockedXor(loc, value, original_value);\n";
            break;
        case SSBOMethod::ATOMIC_EXCHANGE:
            out << "    buffer.InterlockedExchange(loc, value, original_value);\n";
            break;
        case SSBOMethod::ATOMIC_COMPSWAP:
            out << "    buffer.InterlockedCompareExchange(loc, compare_value, value, "
                   "original_value);\n";
            break;
        default:
            UNREACHABLE();
    }
    out << "    return original_value;\n";
}

bool ShaderStorageBlockFunctionHLSL::ShaderStorageBlockFunction::operator<(
    const ShaderStorageBlockFunction &rhs) const
{
    return functionName < rhs.functionName;
}

TString ShaderStorageBlockFunctionHLSL::registerShaderStorageBlockFunction(
    const TType &type,
    SSBOMethod method,
    TLayoutBlockStorage storage,
    bool rowMajor,
    int matrixStride,
    int unsizedArrayStride,
    TIntermSwizzle *swizzleNode)
{
    ShaderStorageBlockFunction ssboFunction;
    ssboFunction.typeString = TypeString(type);
    ssboFunction.method     = method;
    switch (method)
    {
        case SSBOMethod::LOAD:
            ssboFunction.functionName = "_Load_";
            break;
        case SSBOMethod::STORE:
            ssboFunction.functionName = "_Store_";
            break;
        case SSBOMethod::LENGTH:
            ssboFunction.unsizedArrayStride = unsizedArrayStride;
            ssboFunction.functionName       = "_Length_" + str(unsizedArrayStride);
            mRegisteredShaderStorageBlockFunctions.insert(ssboFunction);
            return ssboFunction.functionName;
        case SSBOMethod::ATOMIC_ADD:
            ssboFunction.functionName = "_ssbo_atomicAdd_" + ssboFunction.typeString;
            mRegisteredShaderStorageBlockFunctions.insert(ssboFunction);
            return ssboFunction.functionName;
        case SSBOMethod::ATOMIC_MIN:
            ssboFunction.functionName = "_ssbo_atomicMin_" + ssboFunction.typeString;
            mRegisteredShaderStorageBlockFunctions.insert(ssboFunction);
            return ssboFunction.functionName;
        case SSBOMethod::ATOMIC_MAX:
            ssboFunction.functionName = "_ssbo_atomicMax_" + ssboFunction.typeString;
            mRegisteredShaderStorageBlockFunctions.insert(ssboFunction);
            return ssboFunction.functionName;
        case SSBOMethod::ATOMIC_AND:
            ssboFunction.functionName = "_ssbo_atomicAnd_" + ssboFunction.typeString;
            mRegisteredShaderStorageBlockFunctions.insert(ssboFunction);
            return ssboFunction.functionName;
        case SSBOMethod::ATOMIC_OR:
            ssboFunction.functionName = "_ssbo_atomicOr_" + ssboFunction.typeString;
            mRegisteredShaderStorageBlockFunctions.insert(ssboFunction);
            return ssboFunction.functionName;
        case SSBOMethod::ATOMIC_XOR:
            ssboFunction.functionName = "_ssbo_atomicXor_" + ssboFunction.typeString;
            mRegisteredShaderStorageBlockFunctions.insert(ssboFunction);
            return ssboFunction.functionName;
        case SSBOMethod::ATOMIC_EXCHANGE:
            ssboFunction.functionName = "_ssbo_atomicExchange_" + ssboFunction.typeString;
            mRegisteredShaderStorageBlockFunctions.insert(ssboFunction);
            return ssboFunction.functionName;
        case SSBOMethod::ATOMIC_COMPSWAP:
            ssboFunction.functionName = "_ssbo_atomicCompSwap_" + ssboFunction.typeString;
            mRegisteredShaderStorageBlockFunctions.insert(ssboFunction);
            return ssboFunction.functionName;
        default:
            UNREACHABLE();
    }

    ssboFunction.functionName += ssboFunction.typeString;
    ssboFunction.type = type;
    if (swizzleNode != nullptr)
    {
        ssboFunction.swizzleOffsets   = swizzleNode->getSwizzleOffsets();
        ssboFunction.isDefaultSwizzle = false;
    }
    else
    {
        if (ssboFunction.type.getNominalSize() > 1)
        {
            for (uint8_t index = 0; index < ssboFunction.type.getNominalSize(); index++)
            {
                ssboFunction.swizzleOffsets.push_back(index);
            }
        }
        else
        {
            ssboFunction.swizzleOffsets.push_back(0);
        }

        ssboFunction.isDefaultSwizzle = true;
    }
    ssboFunction.rowMajor     = rowMajor;
    ssboFunction.matrixStride = matrixStride;
    ssboFunction.functionName += "_" + TString(getBlockStorageString(storage));

    if (rowMajor)
    {
        ssboFunction.functionName += "_rm_";
    }
    else
    {
        ssboFunction.functionName += "_cm_";
    }

    for (const int offset : ssboFunction.swizzleOffsets)
    {
        switch (offset)
        {
            case 0:
                ssboFunction.functionName += "x";
                break;
            case 1:
                ssboFunction.functionName += "y";
                break;
            case 2:
                ssboFunction.functionName += "z";
                break;
            case 3:
                ssboFunction.functionName += "w";
                break;
            default:
                UNREACHABLE();
        }
    }

    mRegisteredShaderStorageBlockFunctions.insert(ssboFunction);
    return ssboFunction.functionName;
}

void ShaderStorageBlockFunctionHLSL::shaderStorageBlockFunctionHeader(TInfoSinkBase &out)
{
    for (const ShaderStorageBlockFunction &ssboFunction : mRegisteredShaderStorageBlockFunctions)
    {
        switch (ssboFunction.method)
        {
            case SSBOMethod::LOAD:
            {
                // Function header
                out << ssboFunction.typeString << " " << ssboFunction.functionName
                    << "(RWByteAddressBuffer buffer, uint loc)\n";
                out << "{\n";
                OutputSSBOLoadFunctionBody(out, ssboFunction);
                break;
            }
            case SSBOMethod::STORE:
            {
                // Function header
                out << "void " << ssboFunction.functionName
                    << "(RWByteAddressBuffer buffer, uint loc, " << ssboFunction.typeString
                    << " value)\n";
                out << "{\n";
                OutputSSBOStoreFunctionBody(out, ssboFunction);
                break;
            }
            case SSBOMethod::LENGTH:
            {
                out << "int " << ssboFunction.functionName
                    << "(RWByteAddressBuffer buffer, uint loc)\n";
                out << "{\n";
                OutputSSBOLengthFunctionBody(out, ssboFunction.unsizedArrayStride);
                break;
            }
            case SSBOMethod::ATOMIC_ADD:
            case SSBOMethod::ATOMIC_MIN:
            case SSBOMethod::ATOMIC_MAX:
            case SSBOMethod::ATOMIC_AND:
            case SSBOMethod::ATOMIC_OR:
            case SSBOMethod::ATOMIC_XOR:
            case SSBOMethod::ATOMIC_EXCHANGE:
            {
                out << ssboFunction.typeString << " " << ssboFunction.functionName
                    << "(RWByteAddressBuffer buffer, uint loc, " << ssboFunction.typeString
                    << " value)\n";
                out << "{\n";

                OutputSSBOAtomicMemoryFunctionBody(out, ssboFunction);
                break;
            }
            case SSBOMethod::ATOMIC_COMPSWAP:
            {
                out << ssboFunction.typeString << " " << ssboFunction.functionName
                    << "(RWByteAddressBuffer buffer, uint loc, " << ssboFunction.typeString
                    << " compare_value, " << ssboFunction.typeString << " value)\n";
                out << "{\n";
                OutputSSBOAtomicMemoryFunctionBody(out, ssboFunction);
                break;
            }
            default:
                UNREACHABLE();
        }

        out << "}\n"
               "\n";
    }
}

}  // namespace sh
