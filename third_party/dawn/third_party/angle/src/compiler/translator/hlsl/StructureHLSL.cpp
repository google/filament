//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// StructureHLSL.cpp:
//   HLSL translation of GLSL constructors and structures.
//

#include "compiler/translator/hlsl/StructureHLSL.h"
#include "common/utilities.h"
#include "compiler/translator/Types.h"
#include "compiler/translator/hlsl/OutputHLSL.h"
#include "compiler/translator/hlsl/UtilsHLSL.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

TString Define(const TStructure &structure,
               bool useHLSLRowMajorPacking,
               bool useStd140Packing,
               bool forcePadding,
               Std140PaddingHelper *padHelper)
{
    const TFieldList &fields    = structure.fields();
    const bool isNameless       = (structure.symbolType() == SymbolType::Empty);
    const TString &structName   = QualifiedStructNameString(structure, useHLSLRowMajorPacking,
                                                            useStd140Packing, forcePadding);
    const TString declareString = (isNameless ? "struct" : "struct " + structName);

    TString string;
    string += declareString +
              "\n"
              "{\n";

    size_t memberSize = fields.size();
    for (const TField *field : fields)
    {
        memberSize--;
        const TType &fieldType = *field->type();
        if (!IsSampler(fieldType.getBasicType()))
        {
            const TStructure *fieldStruct = fieldType.getStruct();
            const TString &fieldTypeString =
                fieldStruct ? QualifiedStructNameString(*fieldStruct, useHLSLRowMajorPacking,
                                                        useStd140Packing, false)
                            : TypeString(fieldType);

            if (padHelper)
            {
                string += padHelper->prePaddingString(
                    fieldType, (memberSize != fields.size() - 1) && forcePadding);
            }

            string += "    " + fieldTypeString + " " + DecorateField(field->name(), structure) +
                      ArrayString(fieldType).data() + ";\n";

            if (padHelper)
            {
                string += padHelper->postPaddingString(fieldType, useHLSLRowMajorPacking,
                                                       memberSize == 0, forcePadding);
            }
        }
    }

    // Nameless structs do not finish with a semicolon and newline, to leave room for an instance
    // variable
    string += (isNameless ? "} " : "};\n");

    return string;
}

TString WriteParameterList(const std::vector<TType> &parameters)
{
    TString parameterList;
    for (size_t parameter = 0u; parameter < parameters.size(); parameter++)
    {
        const TType &paramType = parameters[parameter];

        parameterList +=
            TypeString(paramType) + " x" + str(parameter) + ArrayString(paramType).data();

        if (parameter < parameters.size() - 1u)
        {
            parameterList += ", ";
        }
    }
    return parameterList;
}

int GetElementPadding(int elementIndex, int alignment)
{
    const int paddingOffset = elementIndex % alignment;
    return paddingOffset != 0 ? (alignment - paddingOffset) : 0;
}

}  // anonymous namespace

Std140PaddingHelper::Std140PaddingHelper(const std::map<TString, int> &structElementIndexes,
                                         unsigned *uniqueCounter)
    : mPaddingCounter(uniqueCounter), mElementIndex(0), mStructElementIndexes(&structElementIndexes)
{}

Std140PaddingHelper::Std140PaddingHelper(const Std140PaddingHelper &other)
    : mPaddingCounter(other.mPaddingCounter),
      mElementIndex(other.mElementIndex),
      mStructElementIndexes(other.mStructElementIndexes)
{}

Std140PaddingHelper &Std140PaddingHelper::operator=(const Std140PaddingHelper &other)
{
    mPaddingCounter       = other.mPaddingCounter;
    mElementIndex         = other.mElementIndex;
    mStructElementIndexes = other.mStructElementIndexes;
    return *this;
}

TString Std140PaddingHelper::next()
{
    unsigned value = (*mPaddingCounter)++;
    return str(value);
}

int Std140PaddingHelper::prePadding(const TType &type, bool forcePadding)
{
    if (type.getBasicType() == EbtStruct || type.isMatrix() || type.isArray())
    {
        if (forcePadding)
        {
            // Add padding between the structure's members to follow the std140 rules manually.
            const int forcePaddingCount = GetElementPadding(mElementIndex, 4);
            mElementIndex               = 0;
            return forcePaddingCount;
        }
        else
        {
            // no padding needed, HLSL will align the field to a new register
            mElementIndex = 0;
            return 0;
        }
    }

    const GLenum glType     = GLVariableType(type);
    const int numComponents = gl::VariableComponentCount(glType);

    if (numComponents >= 4)
    {
        if (forcePadding)
        {
            // Add padding between the structure's members to follow the std140 rules manually.
            const int forcePaddingCount = GetElementPadding(mElementIndex, 4);
            mElementIndex               = numComponents % 4;
            return forcePaddingCount;
        }
        else
        {
            // no padding needed, HLSL will align the field to a new register
            mElementIndex = 0;
            return 0;
        }
    }

    if (mElementIndex + numComponents > 4)
    {
        if (forcePadding)
        {
            // Add padding between the structure's members to follow the std140 rules manually.
            const int forcePaddingCount = GetElementPadding(mElementIndex, 4);
            mElementIndex               = numComponents;
            return forcePaddingCount;
        }
        else
        {
            // no padding needed, HLSL will align the field to a new register
            mElementIndex = numComponents;
            return 0;
        }
    }

    const int alignment    = numComponents == 3 ? 4 : numComponents;
    const int paddingCount = GetElementPadding(mElementIndex, alignment);

    mElementIndex += paddingCount;
    mElementIndex += numComponents;
    mElementIndex %= 4;

    return paddingCount;
}

TString Std140PaddingHelper::prePaddingString(const TType &type, bool forcePadding)
{
    int paddingCount = prePadding(type, forcePadding);

    TString padding;

    for (int paddingIndex = 0; paddingIndex < paddingCount; paddingIndex++)
    {
        padding += "    float pad_" + next() + ";\n";
    }

    return padding;
}

TString Std140PaddingHelper::postPaddingString(const TType &type,
                                               bool useHLSLRowMajorPacking,
                                               bool isLastElement,
                                               bool forcePadding)
{
    if (!type.isMatrix() && !type.isArray() && type.getBasicType() != EbtStruct)
    {
        if (forcePadding)
        {
            const GLenum glType     = GLVariableType(type);
            const int numComponents = gl::VariableComponentCount(glType);
            if (isLastElement || (numComponents >= 4))
            {
                // If this structure will be used as HLSL StructuredBuffer member's type, in
                // order to follow the std140 rules, add padding at the end of the structure
                // if necessary. Or if the current element straddles a vec4 boundary, add
                // padding to round up the base offset of the next element to the base
                // alignment of a vec4.
                TString forcePaddingStr;
                const int paddingCount = GetElementPadding(mElementIndex, 4);
                for (int paddingIndex = 0; paddingIndex < paddingCount; paddingIndex++)
                {
                    forcePaddingStr += "    float pad_" + next() + ";\n";
                }
                mElementIndex = 0;
                return forcePaddingStr;
            }
        }

        return "";
    }

    int numComponents           = 0;
    const TStructure *structure = type.getStruct();

    if (type.isMatrix())
    {
        // This method can also be called from structureString, which does not use layout
        // qualifiers.
        // Thus, use the method parameter for determining the matrix packing.
        //
        // Note HLSL row major packing corresponds to GL API column-major, and vice-versa, since we
        // wish to always transpose GL matrices to play well with HLSL's matrix array indexing.
        //
        const bool isRowMajorMatrix = !useHLSLRowMajorPacking;
        const GLenum glType         = GLVariableType(type);
        numComponents               = gl::MatrixComponentCount(glType, isRowMajorMatrix);
    }
    else if (structure)
    {
        const TString &structName =
            QualifiedStructNameString(*structure, useHLSLRowMajorPacking, true, false);
        numComponents = mStructElementIndexes->find(structName)->second;

        if (numComponents == 0)
        {
            return "";
        }
    }
    else
    {
        const GLenum glType = GLVariableType(type);
        numComponents       = gl::VariableComponentCount(glType);
    }

    TString padding;
    for (int paddingOffset = numComponents; paddingOffset < 4; paddingOffset++)
    {
        padding += "    float pad_" + next() + ";\n";
    }
    return padding;
}

StructureHLSL::StructureHLSL() : mUniquePaddingCounter(0) {}

Std140PaddingHelper StructureHLSL::getPaddingHelper()
{
    return Std140PaddingHelper(mStd140StructElementIndexes, &mUniquePaddingCounter);
}

TString StructureHLSL::defineQualified(const TStructure &structure,
                                       bool useHLSLRowMajorPacking,
                                       bool useStd140Packing,
                                       bool forcePadding)
{
    if (useStd140Packing)
    {
        Std140PaddingHelper padHelper = getPaddingHelper();
        return Define(structure, useHLSLRowMajorPacking, useStd140Packing, forcePadding,
                      &padHelper);
    }
    else
    {
        return Define(structure, useHLSLRowMajorPacking, useStd140Packing, false, nullptr);
    }
}

TString StructureHLSL::defineNameless(const TStructure &structure)
{
    return Define(structure, false, false, false, nullptr);
}

StructureHLSL::DefinedStructs::iterator StructureHLSL::defineVariants(const TStructure &structure,
                                                                      const TString &name)
{
    ASSERT(mDefinedStructs.find(name) == mDefinedStructs.end());

    for (const TField *field : structure.fields())
    {
        const TType *fieldType = field->type();
        if (fieldType->getBasicType() == EbtStruct)
        {
            ensureStructDefined(*fieldType->getStruct());
        }
    }

    DefinedStructs::iterator addedStruct =
        mDefinedStructs.insert(std::make_pair(name, new TStructProperties())).first;
    // Add element index
    storeStd140ElementIndex(structure, false);
    storeStd140ElementIndex(structure, true);

    const TString &structString = defineQualified(structure, false, false, false);

    ASSERT(std::find(mStructDeclarations.begin(), mStructDeclarations.end(), structString) ==
           mStructDeclarations.end());
    // Add row-major packed struct for interface blocks
    TString rowMajorString = "#pragma pack_matrix(row_major)\n" +
                             defineQualified(structure, true, false, false) +
                             "#pragma pack_matrix(column_major)\n";

    TString std140String         = defineQualified(structure, false, true, false);
    TString std140RowMajorString = "#pragma pack_matrix(row_major)\n" +
                                   defineQualified(structure, true, true, false) +
                                   "#pragma pack_matrix(column_major)\n";

    // Must force to pad the structure's elements for StructuredBuffer's element type, if qualifier
    // of structure is std140.
    TString std140ForcePaddingString         = defineQualified(structure, false, true, true);
    TString std140RowMajorForcePaddingString = "#pragma pack_matrix(row_major)\n" +
                                               defineQualified(structure, true, true, true) +
                                               "#pragma pack_matrix(column_major)\n";

    mStructDeclarations.push_back(structString);
    mStructDeclarations.push_back(rowMajorString);
    mStructDeclarations.push_back(std140String);
    mStructDeclarations.push_back(std140RowMajorString);
    mStructDeclarations.push_back(std140ForcePaddingString);
    mStructDeclarations.push_back(std140RowMajorForcePaddingString);
    return addedStruct;
}

void StructureHLSL::ensureStructDefined(const TStructure &structure)
{
    const TString name = StructNameString(structure);
    if (name == "")
    {
        return;  // Nameless structures are not defined
    }
    if (mDefinedStructs.find(name) == mDefinedStructs.end())
    {
        defineVariants(structure, name);
    }
}

TString StructureHLSL::addStructConstructor(const TStructure &structure)
{
    const TString name = StructNameString(structure);

    if (name == "")
    {
        return TString();  // Nameless structures don't have constructors
    }

    auto definedStruct = mDefinedStructs.find(name);
    if (definedStruct == mDefinedStructs.end())
    {
        definedStruct = defineVariants(structure, name);
    }
    const TString constructorFunctionName = TString(name) + "_ctor";
    TString *constructor                  = &definedStruct->second->constructor;
    if (!constructor->empty())
    {
        return constructorFunctionName;  // Already added
    }
    *constructor += name + " " + constructorFunctionName + "(";

    std::vector<TType> ctorParameters;
    const TFieldList &fields = structure.fields();
    for (const TField *field : fields)
    {
        const TType *fieldType = field->type();
        if (!IsSampler(fieldType->getBasicType()))
        {
            ctorParameters.push_back(*fieldType);
        }
    }
    // Structs that have sampler members should not have constructor calls, and otherwise structs
    // are guaranteed to be non-empty by the grammar. Structs can't contain empty declarations
    // either.
    ASSERT(!ctorParameters.empty());

    *constructor += WriteParameterList(ctorParameters);

    *constructor +=
        ")\n"
        "{\n"
        "    " +
        name + " structure = { ";

    for (size_t parameterIndex = 0u; parameterIndex < ctorParameters.size(); ++parameterIndex)
    {
        *constructor += "x" + str(parameterIndex);
        if (parameterIndex < ctorParameters.size() - 1u)
        {
            *constructor += ", ";
        }
    }
    *constructor +=
        "};\n"
        "    return structure;\n"
        "}\n";

    return constructorFunctionName;
}

TString StructureHLSL::addBuiltInConstructor(const TType &type, const TIntermSequence *parameters)
{
    ASSERT(!type.isArray());
    ASSERT(type.getStruct() == nullptr);
    ASSERT(parameters);

    TType ctorType = type;
    ctorType.setPrecision(EbpHigh);
    ctorType.setQualifier(EvqTemporary);

    const TString constructorFunctionName =
        TString(type.getBuiltInTypeNameString()) + "_ctor" + DisambiguateFunctionName(parameters);
    TString constructor = TypeString(ctorType) + " " + constructorFunctionName + "(";

    std::vector<TType> ctorParameters;
    for (auto parameter : *parameters)
    {
        const TType &paramType = parameter->getAsTyped()->getType();
        ASSERT(!paramType.isArray());
        ctorParameters.push_back(paramType);
    }
    constructor += WriteParameterList(ctorParameters);

    constructor +=
        ")\n"
        "{\n"
        "    return " +
        TypeString(ctorType) + "(";

    if (ctorType.isMatrix() && ctorParameters.size() == 1)
    {
        uint8_t rows           = ctorType.getRows();
        uint8_t cols           = ctorType.getCols();
        const TType &parameter = ctorParameters[0];

        if (parameter.isScalar())
        {
            for (uint8_t col = 0; col < cols; col++)
            {
                for (uint8_t row = 0; row < rows; row++)
                {
                    constructor += TString((row == col) ? "x0" : "0.0");

                    if (row < rows - 1 || col < cols - 1)
                    {
                        constructor += ", ";
                    }
                }
            }
        }
        else if (parameter.isMatrix())
        {
            for (uint8_t col = 0; col < cols; col++)
            {
                for (uint8_t row = 0; row < rows; row++)
                {
                    if (row < parameter.getRows() && col < parameter.getCols())
                    {
                        constructor += TString("x0") + "[" + str(col) + "][" + str(row) + "]";
                    }
                    else
                    {
                        constructor += TString((row == col) ? "1.0" : "0.0");
                    }

                    if (row < rows - 1 || col < cols - 1)
                    {
                        constructor += ", ";
                    }
                }
            }
        }
        else
        {
            ASSERT(rows == 2 && cols == 2 && parameter.isVector() &&
                   parameter.getNominalSize() == 4);

            constructor += "x0";
        }
    }
    else
    {
        size_t remainingComponents = ctorType.getObjectSize();
        size_t parameterIndex      = 0;

        while (remainingComponents > 0)
        {
            const TType &parameter     = ctorParameters[parameterIndex];
            const size_t parameterSize = parameter.getObjectSize();
            bool moreParameters        = parameterIndex + 1 < ctorParameters.size();

            constructor += "x" + str(parameterIndex);

            if (parameter.isScalar())
            {
                remainingComponents -= parameter.getObjectSize();
            }
            else if (parameter.isVector())
            {
                if (remainingComponents == parameterSize || moreParameters)
                {
                    ASSERT(parameterSize <= remainingComponents);
                    remainingComponents -= parameterSize;
                }
                else if (remainingComponents < static_cast<size_t>(parameter.getNominalSize()))
                {
                    switch (remainingComponents)
                    {
                        case 1:
                            constructor += ".x";
                            break;
                        case 2:
                            constructor += ".xy";
                            break;
                        case 3:
                            constructor += ".xyz";
                            break;
                        case 4:
                            constructor += ".xyzw";
                            break;
                        default:
                            UNREACHABLE();
                    }

                    remainingComponents = 0;
                }
                else
                    UNREACHABLE();
            }
            else if (parameter.isMatrix())
            {
                uint8_t column = 0;
                while (remainingComponents > 0 && column < parameter.getCols())
                {
                    constructor += "[" + str(column) + "]";

                    if (remainingComponents < static_cast<size_t>(parameter.getRows()))
                    {
                        switch (remainingComponents)
                        {
                            case 1:
                                constructor += ".x";
                                break;
                            case 2:
                                constructor += ".xy";
                                break;
                            case 3:
                                constructor += ".xyz";
                                break;
                            default:
                                UNREACHABLE();
                        }

                        remainingComponents = 0;
                    }
                    else
                    {
                        remainingComponents -= parameter.getRows();

                        if (remainingComponents > 0)
                        {
                            constructor += ", x" + str(parameterIndex);
                        }
                    }

                    column++;
                }
            }
            else
            {
                UNREACHABLE();
            }

            if (moreParameters)
            {
                parameterIndex++;
            }

            if (remainingComponents)
            {
                constructor += ", ";
            }
        }
    }

    constructor +=
        ");\n"
        "}\n";

    mBuiltInConstructors.insert(constructor);

    return constructorFunctionName;
}

std::string StructureHLSL::structsHeader() const
{
    TInfoSinkBase out;

    for (auto &declaration : mStructDeclarations)
    {
        out << declaration;
    }

    for (auto &structure : mDefinedStructs)
    {
        out << structure.second->constructor;
    }

    for (auto &constructor : mBuiltInConstructors)
    {
        out << constructor;
    }

    return out.str();
}

void StructureHLSL::storeStd140ElementIndex(const TStructure &structure,
                                            bool useHLSLRowMajorPacking)
{
    Std140PaddingHelper padHelper = getPaddingHelper();
    const TFieldList &fields      = structure.fields();

    for (const TField *field : fields)
    {
        padHelper.prePadding(*field->type(), false);
    }

    // Add remaining element index to the global map, for use with nested structs in standard
    // layouts
    const TString &structName =
        QualifiedStructNameString(structure, useHLSLRowMajorPacking, true, false);
    mStd140StructElementIndexes[structName] = padHelper.elementIndex();
}

}  // namespace sh
