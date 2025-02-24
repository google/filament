//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_MODIFYSTRUCT_H_
#define COMPILER_TRANSLATOR_MSL_MODIFYSTRUCT_H_

#include <cstring>
#include <unordered_map>
#include <unordered_set>

#include "compiler/translator/Compiler.h"
#include "compiler/translator/Name.h"
#include "compiler/translator/msl/IdGen.h"
#include "compiler/translator/msl/Layout.h"
#include "compiler/translator/msl/ProgramPrelude.h"
#include "compiler/translator/msl/SymbolEnv.h"

namespace sh
{

enum class ConvertType
{
    OriginalToModified,
    ModifiedToOriginal,
};

// Configures how struct modification is performed.
class ModifyStructConfig
{
  public:
    struct Predicate
    {
        using Func = bool (*)(const TField &);
        static bool False(const TField &) { return false; }
        static bool True(const TField &) { return true; }
    };

    struct SaturateVector
    {
        // Valid return values are [0, 1, 2, 3, 4].
        // If original dim >= return value, the field remains untouched.
        using Func = uint8_t (*)(const TField &);
        static uint8_t DontSaturate(const TField &) { return 0; }
        static uint8_t FullySaturate(const TField &) { return 4; }
    };

  public:
    ModifyStructConfig(ConvertType convertType, bool allowPacking, bool allowPadding)
        : convertType(convertType), allowPacking(allowPacking), allowPadding(allowPadding)
    {}

    // Matrix field is split into multiple fields of row vectors.
    Predicate::Func splitMatrixColumns = Predicate::False;

    // Array fields are split into multiple fields of element type.
    Predicate::Func inlineArray = Predicate::False;

    // Struct fields have their subfields inlined directly.
    Predicate::Func inlineStruct = Predicate::False;

    // Struct fields are modified.
    Predicate::Func recurseStruct = Predicate::False;

    // Vector and scalar bool fields are promoted to uint32_t fields.
    Predicate::Func promoteBoolToUint = Predicate::False;

    // Creates a new structure where scalar or vector fields are saturated vectors.
    // e.g. `float -> float4`.
    // e.g. `float2 -> float4`.
    SaturateVector::Func saturateScalarOrVector = SaturateVector::DontSaturate;

    // Creates a new structure where scalar or vector array fields are saturated.
    // e.g. `float[10] -> float4[10]`
    // e.g. `float2[10] -> float4[10]`
    SaturateVector::Func saturateScalarOrVectorArrays = SaturateVector::DontSaturate;

    // Creates a new structure where matrix fields are row-saturated.
    // e.g. `float2x2 -> float2x4`.
    SaturateVector::Func saturateMatrixRows = SaturateVector::DontSaturate;

    TLayoutBlockStorage initialBlockStorage   = kDefaultLayoutBlockStorage;
    TLayoutMatrixPacking initialMatrixPacking = kDefaultLayoutMatrixPacking;
    ConvertType convertType;
    bool allowPacking;
    bool allowPadding;
    AddressSpace externalAddressSpace;
};

struct ModifiedStructMachinery
{
    const TStructure *modifiedStruct                  = nullptr;
    TIntermFunctionDefinition *funcOriginalToModified = nullptr;
    TIntermFunctionDefinition *funcModifiedToOriginal = nullptr;

    TIntermFunctionDefinition *&getConverter(ConvertType convertType)
    {
        if (convertType == ConvertType::OriginalToModified)
        {
            return funcOriginalToModified;
        }
        else
        {
            return funcModifiedToOriginal;
        }
    }
};

// Indexed by topological order.
class ModifiedStructMachineries
{
  public:
    size_t size() const;
    const ModifiedStructMachinery &at(size_t index) const;
    const ModifiedStructMachinery *find(const TStructure &s) const;
    void insert(const TStructure &s, const ModifiedStructMachinery &machinery);

  private:
    std::unordered_map<const TStructure *, ModifiedStructMachinery> originalToMachinery;
    std::vector<const TStructure *> ordering;
};

// Returns true and `outMachinery` populated if modifications were performed.
// Returns false otherwise.
bool TryCreateModifiedStruct(TCompiler &compiler,
                             SymbolEnv &symbolEnv,
                             IdGen &idGen,
                             const ModifyStructConfig &config,
                             const TStructure &originalStruct,
                             const Name &modifiedStructName,
                             ModifiedStructMachineries &outMachineries,
                             const bool isUBO,
                             const bool allowPadding,
                             const bool useAttributeAliasing);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_MODIFYSTRUCT_H_
