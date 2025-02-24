//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The ValidateVaryingLocations function checks if there exists location conflicts on shader
// varyings.
//

#include "ValidateVaryingLocations.h"

#include "compiler/translator/Diagnostics.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

void error(const TIntermSymbol &symbol, const char *reason, TDiagnostics *diagnostics)
{
    diagnostics->error(symbol.getLine(), reason, symbol.getName().data());
}

int GetStructLocationCount(const TStructure *structure);

int GetFieldLocationCount(const TField *field)
{
    int field_size         = 0;
    const TType *fieldType = field->type();

    if (fieldType->getStruct() != nullptr)
    {
        field_size = GetStructLocationCount(fieldType->getStruct());
    }
    else if (fieldType->isMatrix())
    {
        field_size = fieldType->getNominalSize();
    }
    else
    {
        ASSERT(fieldType->getSecondarySize() == 1);
        field_size = 1;
    }

    if (fieldType->isArray())
    {
        field_size *= fieldType->getArraySizeProduct();
    }

    return field_size;
}

int GetStructLocationCount(const TStructure *structure)
{
    int totalLocation = 0;
    for (const TField *field : structure->fields())
    {
        totalLocation += GetFieldLocationCount(field);
    }
    return totalLocation;
}

int GetInterfaceBlockLocationCount(const TType &varyingType, bool ignoreVaryingArraySize)
{
    int totalLocation = 0;
    for (const TField *field : varyingType.getInterfaceBlock()->fields())
    {
        totalLocation += GetFieldLocationCount(field);
    }

    if (!ignoreVaryingArraySize && varyingType.isArray())
    {
        totalLocation *= varyingType.getArraySizeProduct();
    }
    return totalLocation;
}

int GetLocationCount(const TType &varyingType, bool ignoreVaryingArraySize)
{
    ASSERT(!varyingType.isInterfaceBlock());

    if (varyingType.getStruct() != nullptr)
    {
        int totalLocation = 0;
        for (const TField *field : varyingType.getStruct()->fields())
        {
            const TType *fieldType = field->type();
            ASSERT(fieldType->getStruct() == nullptr && !fieldType->isArray());

            totalLocation += GetFieldLocationCount(field);
        }
        return totalLocation;
    }

    ASSERT(varyingType.isMatrix() || varyingType.getSecondarySize() == 1);
    int elementLocationCount = varyingType.isMatrix() ? varyingType.getNominalSize() : 1;

    // [GL_EXT_shader_io_blocks SPEC Chapter 4.4.1]
    // Geometry shader inputs, tessellation control shader inputs and outputs, and tessellation
    // evaluation inputs all have an additional level of arrayness relative to other shader inputs
    // and outputs. This outer array level is removed from the type before considering how many
    // locations the type consumes.
    if (ignoreVaryingArraySize)
    {
        // Array-of-arrays cannot be inputs or outputs of a geometry shader.
        // (GL_EXT_geometry_shader SPEC issues(5))
        ASSERT(!varyingType.isArrayOfArrays());
        return elementLocationCount;
    }

    return elementLocationCount * varyingType.getArraySizeProduct();
}

bool ShouldIgnoreVaryingArraySize(TQualifier qualifier, GLenum shaderType)
{
    bool isVaryingIn = IsShaderIn(qualifier) && qualifier != EvqPatchIn;

    switch (shaderType)
    {
        case GL_GEOMETRY_SHADER:
        case GL_TESS_EVALUATION_SHADER:
            return isVaryingIn;
        case GL_TESS_CONTROL_SHADER:
            return (IsShaderOut(qualifier) && qualifier != EvqPatchOut) || isVaryingIn;
        default:
            return false;
    }
}

struct SymbolAndField
{
    const TIntermSymbol *symbol;
    const TField *field;
};
using LocationMap = std::map<int, SymbolAndField>;

void MarkVaryingLocations(TDiagnostics *diagnostics,
                          const TIntermSymbol *varying,
                          const TField *field,
                          int location,
                          int elementCount,
                          LocationMap *locationMap)
{
    for (int elementIndex = 0; elementIndex < elementCount; ++elementIndex)
    {
        const int offsetLocation = location + elementIndex;
        auto conflict            = locationMap->find(offsetLocation);
        if (conflict != locationMap->end())
        {
            std::stringstream strstr = sh::InitializeStream<std::stringstream>();
            strstr << "'" << varying->getName();
            if (field)
            {
                strstr << "." << field->name();
            }
            strstr << "' conflicting location with '" << conflict->second.symbol->getName();
            if (conflict->second.field)
            {
                strstr << "." << conflict->second.field->name();
            }
            strstr << "'";
            error(*varying, strstr.str().c_str(), diagnostics);
        }
        else
        {
            (*locationMap)[offsetLocation] = {varying, field};
        }
    }
}

using VaryingVector = std::vector<const TIntermSymbol *>;

void ValidateShaderInterfaceAndAssignLocations(TDiagnostics *diagnostics,
                                               const VaryingVector &varyingVector,
                                               GLenum shaderType)
{
    // Location conflicts can only happen when there are two or more varyings in varyingVector.
    if (varyingVector.size() <= 1)
    {
        return;
    }

    LocationMap locationMap;
    for (const TIntermSymbol *varying : varyingVector)
    {
        const TType &varyingType = varying->getType();
        const int location       = varyingType.getLayoutQualifier().location;
        ASSERT(location >= 0);

        bool ignoreVaryingArraySize =
            ShouldIgnoreVaryingArraySize(varying->getQualifier(), shaderType);

        // A varying is either:
        //
        // - A vector or matrix, which can take a number of contiguous locations
        // - A struct, which also takes a number of contiguous locations
        // - An interface block.
        //
        // Interface blocks can assign arbitrary locations to their fields, for example:
        //
        //     layout(location = 4) in block {
        //         vec4 a;                         // gets location 4
        //         vec4 b;                         // gets location 5
        //         layout(location = 7) vec4 c;    // gets location 7
        //         vec4 d;                         // gets location 8
        //         layout (location = 1) vec4 e;   // gets location 1
        //         vec4 f;                         // gets location 2
        //     };
        //
        // The following code therefore takes two paths.  For non-interface-block types, the number
        // of locations for the varying is calculated (elementCount), and all locations in
        // [location, location + elementCount) are marked as occupied.
        //
        // For interface blocks, a similar algorithm is implemented except each field is
        // individually marked with the location either advancing automatically or taking its value
        // from the field's layout qualifier.

        if (varyingType.isInterfaceBlock())
        {
            int currentLocation       = location;
            bool anyFieldWithLocation = false;

            for (const TField *field : varyingType.getInterfaceBlock()->fields())
            {
                const int fieldLocation = field->type()->getLayoutQualifier().location;
                if (fieldLocation >= 0)
                {
                    currentLocation      = fieldLocation;
                    anyFieldWithLocation = true;
                }

                const int fieldLocationCount = GetFieldLocationCount(field);
                MarkVaryingLocations(diagnostics, varying, field, currentLocation,
                                     fieldLocationCount, &locationMap);

                currentLocation += fieldLocationCount;
            }

            // Array interface blocks can't have location qualifiers on fields.
            ASSERT(ignoreVaryingArraySize || !anyFieldWithLocation || !varyingType.isArray());

            if (!ignoreVaryingArraySize && varyingType.isArray())
            {
                // This is only reached if the varying is an array of interface blocks, with only a
                // layout qualifier on the block itself, for example:
                //
                //     layout(location = 4) in block {
                //         vec4 a;
                //         vec4 b;
                //         vec4 c;
                //         vec4 d;
                //     } instance[N];
                //
                // The locations for instance[0] are already marked by the above code, so we need to
                // further mark locations occupied by instances [1, N).  |currentLocation| is
                // already just past the end of instance[0], which is the beginning of instance[1].
                //
                int remainingLocations = currentLocation * (varyingType.getArraySizeProduct() - 1);
                MarkVaryingLocations(diagnostics, varying, nullptr, currentLocation,
                                     remainingLocations, &locationMap);
            }
        }
        else
        {
            const int elementCount = GetLocationCount(varying->getType(), ignoreVaryingArraySize);
            MarkVaryingLocations(diagnostics, varying, nullptr, location, elementCount,
                                 &locationMap);
        }
    }
}

class ValidateVaryingLocationsTraverser : public TIntermTraverser
{
  public:
    ValidateVaryingLocationsTraverser(GLenum shaderType);
    void validate(TDiagnostics *diagnostics);

  private:
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override;

    VaryingVector mInputVaryingsWithLocation;
    VaryingVector mOutputVaryingsWithLocation;
    GLenum mShaderType;
};

ValidateVaryingLocationsTraverser::ValidateVaryingLocationsTraverser(GLenum shaderType)
    : TIntermTraverser(true, false, false), mShaderType(shaderType)
{}

bool ValidateVaryingLocationsTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    const TIntermSequence &sequence = *(node->getSequence());
    ASSERT(!sequence.empty());

    const TIntermSymbol *symbol = sequence.front()->getAsSymbolNode();
    if (symbol == nullptr)
    {
        return false;
    }

    if (symbol->variable().symbolType() == SymbolType::Empty)
    {
        return false;
    }

    // Collect varyings that have explicit 'location' qualifiers.
    const TQualifier qualifier = symbol->getQualifier();
    if (symbol->getType().getLayoutQualifier().location != -1)
    {
        if (IsVaryingIn(qualifier))
        {
            mInputVaryingsWithLocation.push_back(symbol);
        }
        else if (IsVaryingOut(qualifier))
        {
            mOutputVaryingsWithLocation.push_back(symbol);
        }
    }

    return false;
}

bool ValidateVaryingLocationsTraverser::visitFunctionDefinition(Visit visit,
                                                                TIntermFunctionDefinition *node)
{
    // We stop traversing function definitions because varyings cannot be defined in a function.
    return false;
}

void ValidateVaryingLocationsTraverser::validate(TDiagnostics *diagnostics)
{
    ASSERT(diagnostics);

    ValidateShaderInterfaceAndAssignLocations(diagnostics, mInputVaryingsWithLocation, mShaderType);
    ValidateShaderInterfaceAndAssignLocations(diagnostics, mOutputVaryingsWithLocation,
                                              mShaderType);
}

}  // anonymous namespace

unsigned int CalculateVaryingLocationCount(const TType &varyingType, GLenum shaderType)
{
    const TQualifier qualifier        = varyingType.getQualifier();
    const bool ignoreVaryingArraySize = ShouldIgnoreVaryingArraySize(qualifier, shaderType);

    if (varyingType.isInterfaceBlock())
    {
        return GetInterfaceBlockLocationCount(varyingType, ignoreVaryingArraySize);
    }

    return GetLocationCount(varyingType, ignoreVaryingArraySize);
}

bool ValidateVaryingLocations(TIntermBlock *root, TDiagnostics *diagnostics, GLenum shaderType)
{
    ValidateVaryingLocationsTraverser varyingValidator(shaderType);
    root->traverse(&varyingValidator);
    int numErrorsBefore = diagnostics->numErrors();
    varyingValidator.validate(diagnostics);
    return (diagnostics->numErrors() == numErrorsBefore);
}

}  // namespace sh
