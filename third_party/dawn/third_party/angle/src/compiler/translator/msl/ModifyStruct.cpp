//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <algorithm>
#include <cstring>
#include <functional>
#include <numeric>
#include <unordered_map>
#include <unordered_set>

#include "compiler/translator/Compiler.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/msl/AstHelpers.h"
#include "compiler/translator/msl/ModifyStruct.h"
#include "compiler/translator/msl/TranslatorMSL.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

size_t ModifiedStructMachineries::size() const
{
    return ordering.size();
}

const ModifiedStructMachinery &ModifiedStructMachineries::at(size_t index) const
{
    ASSERT(index < size());
    const TStructure *s              = ordering[index];
    const ModifiedStructMachinery *m = find(*s);
    ASSERT(m);
    return *m;
}

const ModifiedStructMachinery *ModifiedStructMachineries::find(const TStructure &s) const
{
    auto iter = originalToMachinery.find(&s);
    if (iter == originalToMachinery.end())
    {
        return nullptr;
    }
    return &iter->second;
}

void ModifiedStructMachineries::insert(const TStructure &s,
                                       const ModifiedStructMachinery &machinery)
{
    ASSERT(!find(s));
    originalToMachinery[&s] = machinery;
    ordering.push_back(&s);
}

////////////////////////////////////////////////////////////////////////////////

namespace
{

TIntermTyped &Flatten(SymbolEnv &symbolEnv, TIntermTyped &node)
{
    auto &type = node.getType();
    ASSERT(type.isArray());

    auto &retType = InnermostType(type);
    retType.makeArray(1);

    return symbolEnv.callFunctionOverload(Name("flatten"), retType, *new TIntermSequence{&node});
}

struct FlattenArray
{};

struct PathItem
{
    enum class Type
    {
        Field,         // Struct field indexing.
        Index,         // Array, vector, or matrix indexing.
        FlattenArray,  // Array of any rank -> pointer of innermost type.
    };

    PathItem(const TField &field) : field(&field), type(Type::Field) {}
    PathItem(int index) : index(index), type(Type::Index) {}
    PathItem(unsigned index) : PathItem(static_cast<int>(index)) {}
    PathItem(FlattenArray flatten) : type(Type::FlattenArray) {}

    union
    {
        const TField *field;
        int index;
    };
    Type type;
};

TIntermTyped &BuildPathAccess(SymbolEnv &symbolEnv,
                              const TVariable &var,
                              const std::vector<PathItem> &path)
{
    TIntermTyped *curr = new TIntermSymbol(&var);
    for (const PathItem &item : path)
    {
        switch (item.type)
        {
            case PathItem::Type::Field:
                curr = &AccessField(*curr, Name(*item.field));
                break;
            case PathItem::Type::Index:
                curr = &AccessIndex(*curr, item.index);
                break;
            case PathItem::Type::FlattenArray:
            {
                curr = &Flatten(symbolEnv, *curr);
            }
            break;
        }
    }
    return *curr;
}

////////////////////////////////////////////////////////////////////////////////

using OriginalParam = const TVariable &;
using ModifiedParam = const TVariable &;

using OriginalAccess = TIntermTyped;
using ModifiedAccess = TIntermTyped;

struct Access
{
    OriginalAccess &original;
    ModifiedAccess &modified;

    struct Env
    {
        const ConvertType type;
    };
};

using ConversionFunc = std::function<Access(Access::Env &, OriginalAccess &, ModifiedAccess &)>;

class ConvertStructState : angle::NonCopyable
{
  private:
    struct ConversionInfo
    {
        ConversionFunc stdFunc;
        const TFunction *astFunc;
        std::vector<PathItem> pathItems;
        Name modifiedFieldName;
    };

  public:
    ConvertStructState(TCompiler &compiler,
                       SymbolEnv &symbolEnv,
                       IdGen &idGen,
                       const ModifyStructConfig &config,
                       ModifiedStructMachineries &outMachineries,
                       const bool isUBO,
                       const bool useAttributeAliasing)
        : mCompiler(compiler),
          config(config),
          symbolEnv(symbolEnv),
          modifiedFields(*new TFieldList()),
          symbolTable(symbolEnv.symbolTable()),
          idGen(idGen),
          outMachineries(outMachineries),
          isUBO(isUBO),
          useAttributeAliasing(useAttributeAliasing)
    {}

    ~ConvertStructState()
    {
    }

    void publish(const TStructure &originalStruct, const Name &modifiedStructName)
    {
        const bool isOriginalToModified = config.convertType == ConvertType::OriginalToModified;

        auto &modifiedStruct = *new TStructure(&symbolTable, modifiedStructName.rawName(),
                                               &modifiedFields, modifiedStructName.symbolType());

        auto &func = *new TFunction(
            &symbolTable,
            idGen.createNewName(isOriginalToModified ? "originalToModified" : "modifiedToOriginal")
                .rawName(),
            SymbolType::AngleInternal, new TType(TBasicType::EbtVoid), false);

        OriginalParam originalParam =
            CreateInstanceVariable(symbolTable, originalStruct, Name("original"));
        ModifiedParam modifiedParam =
            CreateInstanceVariable(symbolTable, modifiedStruct, Name("modified"));

        symbolEnv.markAsReference(originalParam, AddressSpace::Thread);
        symbolEnv.markAsReference(modifiedParam, config.externalAddressSpace);
        if (isOriginalToModified)
        {
            func.addParameter(&originalParam);
            func.addParameter(&modifiedParam);
        }
        else
        {
            func.addParameter(&modifiedParam);
            func.addParameter(&originalParam);
        }

        TIntermBlock &body = *new TIntermBlock();

        Access::Env env{config.convertType};

        for (ConversionInfo &info : conversionInfos)
        {
            auto convert = [&](OriginalAccess &original, ModifiedAccess &modified) {
                if (info.astFunc)
                {
                    ASSERT(!info.stdFunc);
                    TIntermTyped &src  = isOriginalToModified ? modified : original;
                    TIntermTyped &dest = isOriginalToModified ? original : modified;
                    body.appendStatement(TIntermAggregate::CreateFunctionCall(
                        *info.astFunc, new TIntermSequence{&dest, &src}));
                }
                else
                {
                    ASSERT(info.stdFunc);
                    Access access      = info.stdFunc(env, original, modified);
                    TIntermTyped &src  = isOriginalToModified ? access.original : access.modified;
                    TIntermTyped &dest = isOriginalToModified ? access.modified : access.original;
                    body.appendStatement(new TIntermBinary(TOperator::EOpAssign, &dest, &src));
                }
            };
            OriginalAccess *original = &BuildPathAccess(symbolEnv, originalParam, info.pathItems);
            ModifiedAccess *modified = &AccessField(modifiedParam, info.modifiedFieldName);
            if (useAttributeAliasing)
            {
                std::ostringstream aliasedName;
                aliasedName << "ANGLE_ALIASED_" << info.modifiedFieldName;

                TType *placeholderType = new TType(modified->getType());
                placeholderType->setQualifier(EvqSpecConst);

                modified = new TIntermSymbol(
                    new TVariable(&symbolTable, sh::ImmutableString(aliasedName.str()),
                                  placeholderType, SymbolType::AngleInternal));
            }
            const TType ot = original->getType();
            const TType mt = modified->getType();
            ASSERT(ot.isArray() == mt.isArray());

            // Clip distance output uses float[n] type, so the field must be assigned per-element
            // when filling the modified struct. Explicit path name is used because original types
            // are not available here.
            if (ot.isArray() &&
                (ot.getLayoutQualifier().matrixPacking == EmpRowMajor || ot != mt ||
                 info.modifiedFieldName == Name("gl_ClipDistance", SymbolType::BuiltIn)))
            {
                ASSERT(ot.getArraySizes() == mt.getArraySizes());
                if (ot.isArrayOfArrays())
                {
                    original = &Flatten(symbolEnv, *original);
                    modified = &Flatten(symbolEnv, *modified);
                }
                const int volume = static_cast<int>(ot.getArraySizeProduct());
                for (int i = 0; i < volume; ++i)
                {
                    if (i != 0)
                    {
                        original = original->deepCopy();
                        modified = modified->deepCopy();
                    }
                    OriginalAccess &o = AccessIndex(*original, i);
                    OriginalAccess &m = AccessIndex(*modified, i);
                    convert(o, m);
                }
            }
            else
            {
                convert(*original, *modified);
            }
        }

        auto *funcProto = new TIntermFunctionPrototype(&func);
        auto *funcDef   = new TIntermFunctionDefinition(funcProto, &body);

        ModifiedStructMachinery machinery;
        machinery.modifiedStruct                   = &modifiedStruct;
        machinery.getConverter(config.convertType) = funcDef;

        outMachineries.insert(originalStruct, machinery);
    }

    ImmutableString rootFieldName() const
    {
        if (!pathItems.empty())
        {
            if (pathItems[0].type == PathItem::Type::Field)
            {
                return pathItems[0].field->name();
            }
        }
        UNREACHABLE();
        return kEmptyImmutableString;
    }

    void pushPath(PathItem const &item) { pathItems.push_back(item); }

    void popPath()
    {
        ASSERT(!pathItems.empty());
        pathItems.pop_back();
        if (pathItems.empty())
        {
            // Next push will start a new root output variable to linearize.
            mSubfieldIndex = 0;
        }
    }

    void finalize(const bool allowPadding)
    {
        ASSERT(!finalized);
        finalized = true;
        introducePacking();
        ASSERT(metalLayoutTotal == Layout::Identity());
        // Only pad substructs. We don't want to pad the structure that contains all the UBOs, only
        // individual UBOs.
        if (allowPadding)
            introducePadding();
    }

    void addModifiedField(const TField &field,
                          TType &newType,
                          TLayoutBlockStorage storage,
                          TLayoutMatrixPacking packing,
                          const AddressSpace *addressSpace)
    {
        TLayoutQualifier layoutQualifier = newType.getLayoutQualifier();
        layoutQualifier.blockStorage     = storage;
        layoutQualifier.matrixPacking    = packing;
        newType.setLayoutQualifier(layoutQualifier);
        sh::ImmutableString newName  = field.name();
        sh::SymbolType newSymbolType = field.symbolType();
        if (pathItems.size() > 1)
        {
            // Current state is linearizing a root input field into multiple modified fields. The
            // new fields need unique names. Generate the new names into AngleInternal namespace.
            // The user could choose a clashing name in UserDefined namespace.
            newSymbolType = SymbolType::AngleInternal;
            // The user specified root field name is currently used as the basis for the MSL vs-fs
            // interface matching. The field linearization itself is deterministic, so subfield
            // index is sufficient to define all the entries in MSL interface in all the compatible
            // VS and FS MSL programs.
            newName = BuildConcatenatedImmutableString(rootFieldName(), '_', mSubfieldIndex);
            ++mSubfieldIndex;
        }
        TField *modifiedField = new TField(&newType, newName, field.line(), newSymbolType);
        if (addressSpace)
        {
            symbolEnv.markAsPointer(*modifiedField, *addressSpace);
        }
        if (symbolEnv.isUBO(field))
        {
            symbolEnv.markAsUBO(*modifiedField);
        }
        modifiedFields.push_back(modifiedField);
    }

    void addConversion(const ConversionFunc &func)
    {
        ASSERT(!modifiedFields.empty());
        conversionInfos.push_back({func, nullptr, pathItems, Name(*modifiedFields.back())});
    }

    void addConversion(const TFunction &func)
    {
        ASSERT(!modifiedFields.empty());
        conversionInfos.push_back({{}, &func, pathItems, Name(*modifiedFields.back())});
    }

    bool hasPacking() const { return containsPacked; }

    bool hasPadding() const { return padFieldCount > 0; }

    bool recurse(const TStructure &structure,
                 ModifiedStructMachinery &outMachinery,
                 const bool isUBORecurse)
    {
        const ModifiedStructMachinery *m = outMachineries.find(structure);
        if (m == nullptr)
        {
            TranslatorMetalReflection *reflection = mtl::getTranslatorMetalReflection(&mCompiler);
            reflection->addOriginalName(structure.uniqueId().get(), structure.name().data());
            const Name name = idGen.createNewName(structure.name().data());
            if (!TryCreateModifiedStruct(mCompiler, symbolEnv, idGen, config, structure, name,
                                         outMachineries, isUBORecurse, config.allowPadding, false))
            {
                return false;
            }
            m = outMachineries.find(structure);
            ASSERT(m);
        }
        outMachinery = *m;
        return true;
    }

    bool getIsUBO() const { return isUBO; }

  private:
    void addPadding(size_t padAmount, bool updateLayout)
    {
        if (padAmount == 0)
        {
            return;
        }

        const size_t begin = modifiedFields.size();

        // Iteratively adding in scalar or vector padding because some struct types will not
        // allow matrix or array members.
        while (padAmount > 0)
        {
            TType *padType;
            if (padAmount >= 16)
            {
                padAmount -= 16;
                padType = new TType(TBasicType::EbtFloat, 4);
            }
            else if (padAmount >= 8)
            {
                padAmount -= 8;
                padType = new TType(TBasicType::EbtFloat, 2);
            }
            else if (padAmount >= 4)
            {
                padAmount -= 4;
                padType = new TType(TBasicType::EbtFloat);
            }
            else if (padAmount >= 2)
            {
                padAmount -= 2;
                padType = new TType(TBasicType::EbtBool, 2);
            }
            else
            {
                ASSERT(padAmount == 1);
                padAmount -= 1;
                padType = new TType(TBasicType::EbtBool);
            }

            if (padType->getBasicType() != EbtBool)
            {
                padType->setPrecision(EbpLow);
            }

            if (updateLayout)
            {
                metalLayoutTotal += MetalLayoutOf(*padType);
            }

            const Name name = idGen.createNewName("pad");
            modifiedFields.push_back(
                new TField(padType, name.rawName(), kNoSourceLoc, name.symbolType()));
            ++padFieldCount;
        }

        std::reverse(modifiedFields.begin() + begin, modifiedFields.end());
    }

    void introducePacking()
    {
        if (!config.allowPacking)
        {
            return;
        }

        auto setUnpackedStorage = [](TType &type) {
            TLayoutBlockStorage storage = type.getLayoutQualifier().blockStorage;
            switch (storage)
            {
                case TLayoutBlockStorage::EbsShared:
                    storage = TLayoutBlockStorage::EbsStd140;
                    break;
                case TLayoutBlockStorage::EbsPacked:
                    storage = TLayoutBlockStorage::EbsStd430;
                    break;
                case TLayoutBlockStorage::EbsStd140:
                case TLayoutBlockStorage::EbsStd430:
                case TLayoutBlockStorage::EbsUnspecified:
                    break;
            }
            SetBlockStorage(type, storage);
        };

        Layout glslLayoutTotal = Layout::Identity();
        const size_t size      = modifiedFields.size();

        for (size_t i = 0; i < size; ++i)
        {
            TField &curr           = *modifiedFields[i];
            TType &currType        = *curr.type();
            const bool canBePacked = CanBePacked(currType);

            auto dontPack = [&]() {
                if (canBePacked)
                {
                    setUnpackedStorage(currType);
                }
                glslLayoutTotal += GlslLayoutOf(currType);
            };

            if (!CanBePacked(currType))
            {
                dontPack();
                continue;
            }

            const Layout packedGlslLayout           = GlslLayoutOf(currType);
            const TLayoutBlockStorage packedStorage = currType.getLayoutQualifier().blockStorage;
            setUnpackedStorage(currType);
            const Layout unpackedGlslLayout = GlslLayoutOf(currType);
            SetBlockStorage(currType, packedStorage);

            ASSERT(packedGlslLayout.sizeOf <= unpackedGlslLayout.sizeOf);
            if (packedGlslLayout.sizeOf == unpackedGlslLayout.sizeOf)
            {
                dontPack();
                continue;
            }

            const size_t j = i + 1;
            if (j == size)
            {
                dontPack();
                break;
            }

            const size_t pad            = unpackedGlslLayout.sizeOf - packedGlslLayout.sizeOf;
            const TField &next          = *modifiedFields[j];
            const Layout nextGlslLayout = GlslLayoutOf(*next.type());

            if (pad < nextGlslLayout.sizeOf)
            {
                dontPack();
                continue;
            }

            symbolEnv.markAsPacked(curr);
            glslLayoutTotal += packedGlslLayout;
            containsPacked = true;
        }
    }

    void introducePadding()
    {
        if (!config.allowPadding)
        {
            return;
        }

        MetalLayoutOfConfig layoutConfig;
        layoutConfig.disablePacking             = !config.allowPacking;
        layoutConfig.assumeStructsAreTailPadded = true;

        TFieldList fields = std::move(modifiedFields);
        ASSERT(!fields.empty());  // GLSL requires at least one member.

        const TField *const first = fields.front();

        for (TField *field : fields)
        {
            const TType &type = *field->type();

            const Layout glslLayout  = GlslLayoutOf(type);
            const Layout metalLayout = MetalLayoutOf(type, layoutConfig);

            size_t prePadAmount = 0;
            if (glslLayout.alignOf > metalLayout.alignOf && field != first)
            {
                const size_t prePaddedSize = metalLayoutTotal.sizeOf;
                metalLayoutTotal.requireAlignment(glslLayout.alignOf, true);
                const size_t paddedSize = metalLayoutTotal.sizeOf;
                prePadAmount            = paddedSize - prePaddedSize;
                metalLayoutTotal += metalLayout;
                addPadding(prePadAmount, false);  // Note: requireAlignment() already updated layout
            }
            else
            {
                metalLayoutTotal += metalLayout;
            }

            modifiedFields.push_back(field);

            if (glslLayout.sizeOf > metalLayout.sizeOf && field != fields.back())
            {
                const bool updateLayout = true;  // XXX: Correct?
                const size_t padAmount  = glslLayout.sizeOf - metalLayout.sizeOf;
                addPadding(padAmount, updateLayout);
            }
        }
    }

  public:
    TCompiler &mCompiler;
    const ModifyStructConfig &config;
    SymbolEnv &symbolEnv;

  private:
    TFieldList &modifiedFields;
    Layout metalLayoutTotal = Layout::Identity();
    size_t padFieldCount    = 0;
    bool containsPacked     = false;
    bool finalized          = false;

    std::vector<PathItem> pathItems;

    int mSubfieldIndex = 0;

    std::vector<ConversionInfo> conversionInfos;
    TSymbolTable &symbolTable;
    IdGen &idGen;
    ModifiedStructMachineries &outMachineries;
    const bool isUBO;
    const bool useAttributeAliasing;
};

////////////////////////////////////////////////////////////////////////////////

using ModifyFunc = bool (*)(ConvertStructState &state,
                            const TField &field,
                            const TLayoutBlockStorage storage,
                            const TLayoutMatrixPacking packing);

bool ModifyRecursive(ConvertStructState &state,
                     const TField &field,
                     const TLayoutBlockStorage storage,
                     const TLayoutMatrixPacking packing);

bool IdentityModify(ConvertStructState &state,
                    const TField &field,
                    const TLayoutBlockStorage storage,
                    const TLayoutMatrixPacking packing)
{
    const TType &type = *field.type();
    state.addModifiedField(field, CloneType(type), storage, packing, nullptr);
    state.addConversion([=](Access::Env &, OriginalAccess &o, ModifiedAccess &m) {
        return Access{o, m};
    });
    return false;
}

bool InlineStruct(ConvertStructState &state,
                  const TField &field,
                  const TLayoutBlockStorage storage,
                  const TLayoutMatrixPacking packing)
{
    const TType &type              = *field.type();
    const TStructure *substructure = state.symbolEnv.remap(type.getStruct());
    if (!substructure)
    {
        return false;
    }
    if (type.isArray())
    {
        return false;
    }
    if (!state.config.inlineStruct(field))
    {
        return false;
    }

    const TFieldList &subfields = substructure->fields();
    for (const TField *subfield : subfields)
    {
        const TType &subtype                  = *subfield->type();
        const TLayoutBlockStorage substorage  = Overlay(storage, subtype);
        const TLayoutMatrixPacking subpacking = Overlay(packing, subtype);
        ModifyRecursive(state, *subfield, substorage, subpacking);
    }

    return true;
}

bool RecurseStruct(ConvertStructState &state,
                   const TField &field,
                   const TLayoutBlockStorage storage,
                   const TLayoutMatrixPacking packing)
{
    const TType &type              = *field.type();
    const TStructure *substructure = state.symbolEnv.remap(type.getStruct());
    if (!substructure)
    {
        return false;
    }
    if (!state.config.recurseStruct(field))
    {
        return false;
    }

    ModifiedStructMachinery machinery;
    if (!state.recurse(*substructure, machinery, state.getIsUBO()))
    {
        return false;
    }

    TType &newType = *new TType(machinery.modifiedStruct, false);
    if (type.isArray())
    {
        newType.makeArrays(type.getArraySizes());
    }

    TIntermFunctionDefinition *converter = machinery.getConverter(state.config.convertType);
    ASSERT(converter);

    state.addModifiedField(field, newType, storage, packing, state.symbolEnv.isPointer(field));
    if (state.symbolEnv.isPointer(field))
    {
        state.symbolEnv.removePointer(field);
    }
    state.addConversion(*converter->getFunction());

    return true;
}

bool SplitMatrixColumns(ConvertStructState &state,
                        const TField &field,
                        const TLayoutBlockStorage storage,
                        const TLayoutMatrixPacking packing)
{
    const TType &type = *field.type();
    if (!type.isMatrix())
    {
        return false;
    }

    if (!state.config.splitMatrixColumns(field))
    {
        return false;
    }

    const uint8_t cols = type.getCols();
    TType &rowType     = DropColumns(type);

    for (uint8_t c = 0; c < cols; ++c)
    {
        state.pushPath(c);

        state.addModifiedField(field, rowType, storage, packing, state.symbolEnv.isPointer(field));
        if (state.symbolEnv.isPointer(field))
        {
            state.symbolEnv.removePointer(field);
        }
        state.addConversion([=](Access::Env &, OriginalAccess &o, ModifiedAccess &m) {
            return Access{o, m};
        });

        state.popPath();
    }

    return true;
}

bool SaturateMatrixRows(ConvertStructState &state,
                        const TField &field,
                        const TLayoutBlockStorage storage,
                        const TLayoutMatrixPacking packing)
{
    const TType &type = *field.type();
    if (!type.isMatrix())
    {
        return false;
    }
    const bool isRowMajor    = type.getLayoutQualifier().matrixPacking == EmpRowMajor;
    const uint8_t rows       = type.getRows();
    const uint8_t saturation = state.config.saturateMatrixRows(field);
    if (saturation <= rows)
    {
        return false;
    }

    const uint8_t cols = type.getCols();
    TType &satType     = SetMatrixRowDim(type, saturation);
    state.addModifiedField(field, satType, storage, packing, state.symbolEnv.isPointer(field));
    if (state.symbolEnv.isPointer(field))
    {
        state.symbolEnv.removePointer(field);
    }

    for (uint8_t c = 0; c < cols; ++c)
    {
        for (uint8_t r = 0; r < rows; ++r)
        {
            state.addConversion([=](Access::Env &, OriginalAccess &o, ModifiedAccess &m) {
                uint8_t firstModifiedIndex  = isRowMajor ? r : c;
                uint8_t secondModifiedIndex = isRowMajor ? c : r;
                auto &o_                    = AccessIndex(AccessIndex(o, c), r);
                auto &m_ = AccessIndex(AccessIndex(m, firstModifiedIndex), secondModifiedIndex);
                return Access{o_, m_};
            });
        }
    }

    return true;
}

bool TestBoolToUint(ConvertStructState &state, const TField &field)
{
    if (field.type()->getBasicType() != TBasicType::EbtBool)
    {
        return false;
    }
    if (!state.config.promoteBoolToUint(field))
    {
        return false;
    }
    return true;
}

Access ConvertBoolToUint(ConvertType convertType, OriginalAccess &o, ModifiedAccess &m)
{
    auto coerce = [](TIntermTyped &to, TIntermTyped &from) -> TIntermTyped & {
        return *TIntermAggregate::CreateConstructor(to.getType(), new TIntermSequence{&from});
    };
    switch (convertType)
    {
        case ConvertType::OriginalToModified:
            return Access{coerce(m, o), m};
        case ConvertType::ModifiedToOriginal:
            return Access{o, coerce(o, m)};
    }
}

bool SaturateScalarOrVectorCommon(ConvertStructState &state,
                                  const TField &field,
                                  const TLayoutBlockStorage storage,
                                  const TLayoutMatrixPacking packing,
                                  const bool array)
{
    const TType &type = *field.type();
    if (type.isArray() != array)
    {
        return false;
    }
    if (!((type.isRank0() && HasScalarBasicType(type)) || type.isVector()))
    {
        return false;
    }
    const auto saturator =
        array ? state.config.saturateScalarOrVectorArrays : state.config.saturateScalarOrVector;
    const uint8_t dim        = type.getNominalSize();
    const uint8_t saturation = saturator(field);
    if (saturation <= dim)
    {
        return false;
    }

    TType &satType        = SetVectorDim(type, saturation);
    const bool boolToUint = TestBoolToUint(state, field);
    if (boolToUint)
    {
        satType.setBasicType(TBasicType::EbtUInt);
    }
    state.addModifiedField(field, satType, storage, packing, state.symbolEnv.isPointer(field));
    if (state.symbolEnv.isPointer(field))
    {
        state.symbolEnv.removePointer(field);
    }

    for (uint8_t d = 0; d < dim; ++d)
    {
        state.addConversion([=](Access::Env &env, OriginalAccess &o, ModifiedAccess &m) {
            auto &o_ = dim > 1 ? AccessIndex(o, d) : o;
            auto &m_ = AccessIndex(m, d);
            if (boolToUint)
            {
                return ConvertBoolToUint(env.type, o_, m_);
            }
            else
            {
                return Access{o_, m_};
            }
        });
    }

    return true;
}

bool SaturateScalarOrVectorArrays(ConvertStructState &state,
                                  const TField &field,
                                  const TLayoutBlockStorage storage,
                                  const TLayoutMatrixPacking packing)
{
    return SaturateScalarOrVectorCommon(state, field, storage, packing, true);
}

bool SaturateScalarOrVector(ConvertStructState &state,
                            const TField &field,
                            const TLayoutBlockStorage storage,
                            const TLayoutMatrixPacking packing)
{
    return SaturateScalarOrVectorCommon(state, field, storage, packing, false);
}

bool PromoteBoolToUint(ConvertStructState &state,
                       const TField &field,
                       const TLayoutBlockStorage storage,
                       const TLayoutMatrixPacking packing)
{
    if (!TestBoolToUint(state, field))
    {
        return false;
    }

    auto &promotedType = CloneType(*field.type());
    promotedType.setBasicType(TBasicType::EbtUInt);
    state.addModifiedField(field, promotedType, storage, packing, state.symbolEnv.isPointer(field));
    if (state.symbolEnv.isPointer(field))
    {
        state.symbolEnv.removePointer(field);
    }

    state.addConversion([=](Access::Env &env, OriginalAccess &o, ModifiedAccess &m) {
        return ConvertBoolToUint(env.type, o, m);
    });

    return true;
}

bool ModifyCommon(ConvertStructState &state,
                  const TField &field,
                  const TLayoutBlockStorage storage,
                  const TLayoutMatrixPacking packing)
{
    ModifyFunc funcs[] = {
        InlineStruct,                  //
        RecurseStruct,                 //
        SplitMatrixColumns,            //
        SaturateMatrixRows,            //
        SaturateScalarOrVectorArrays,  //
        SaturateScalarOrVector,        //
        PromoteBoolToUint,             //
    };

    for (ModifyFunc func : funcs)
    {
        if (func(state, field, storage, packing))
        {
            return true;
        }
    }

    return IdentityModify(state, field, storage, packing);
}

bool InlineArray(ConvertStructState &state,
                 const TField &field,
                 const TLayoutBlockStorage storage,
                 const TLayoutMatrixPacking packing)
{
    const TType &type = *field.type();
    if (!type.isArray())
    {
        return false;
    }
    if (!state.config.inlineArray(field))
    {
        return false;
    }

    const unsigned volume = type.getArraySizeProduct();
    const bool isMultiDim = type.isArrayOfArrays();

    auto &innermostType = InnermostType(type);

    if (isMultiDim)
    {
        state.pushPath(FlattenArray());
    }

    for (unsigned i = 0; i < volume; ++i)
    {
        state.pushPath(i);
        TType setType(innermostType);
        if (setType.getLayoutQualifier().locationsSpecified)
        {
            TLayoutQualifier qualifier(innermostType.getLayoutQualifier());
            qualifier.location           = innermostType.getLayoutQualifier().location + i;
            qualifier.locationsSpecified = 1;
            setType.setLayoutQualifier(qualifier);
        }
        const TField innermostField(&setType, field.name(), field.line(), field.symbolType());
        ModifyCommon(state, innermostField, storage, packing);
        state.popPath();
    }

    if (isMultiDim)
    {
        state.popPath();
    }

    return true;
}

bool ModifyRecursive(ConvertStructState &state,
                     const TField &field,
                     const TLayoutBlockStorage storage,
                     const TLayoutMatrixPacking packing)
{
    state.pushPath(field);

    bool modified;
    if (InlineArray(state, field, storage, packing))
    {
        modified = true;
    }
    else
    {
        modified = ModifyCommon(state, field, storage, packing);
    }

    state.popPath();

    return modified;
}

}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

bool sh::TryCreateModifiedStruct(TCompiler &compiler,
                                 SymbolEnv &symbolEnv,
                                 IdGen &idGen,
                                 const ModifyStructConfig &config,
                                 const TStructure &originalStruct,
                                 const Name &modifiedStructName,
                                 ModifiedStructMachineries &outMachineries,
                                 const bool isUBO,
                                 const bool allowPadding,
                                 const bool useAttributeAliasing)
{
    ConvertStructState state(compiler, symbolEnv, idGen, config, outMachineries, isUBO,
                             useAttributeAliasing);
    size_t identicalFieldCount = 0;

    const TFieldList &originalFields = originalStruct.fields();
    for (TField *originalField : originalFields)
    {
        const TType &originalType          = *originalField->type();
        const TLayoutBlockStorage storage  = Overlay(config.initialBlockStorage, originalType);
        const TLayoutMatrixPacking packing = Overlay(config.initialMatrixPacking, originalType);
        if (!ModifyRecursive(state, *originalField, storage, packing))
        {
            ++identicalFieldCount;
        }
    }

    state.finalize(allowPadding);

    if (identicalFieldCount == originalFields.size() && !state.hasPacking() &&
        !state.hasPadding() && !useAttributeAliasing)
    {
        return false;
    }
    state.publish(originalStruct, modifiedStructName);

    return true;
}
