//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_SYMBOLENV_H_
#define COMPILER_TRANSLATOR_MSL_SYMBOLENV_H_

#include <unordered_set>

#include "compiler/translator/Compiler.h"
#include "compiler/translator/Name.h"
#include "compiler/translator/Types.h"
#include "compiler/translator/msl/Reference.h"

namespace sh
{

enum class AddressSpace
{
    Constant,
    Device,
    Thread,
};

char const *toString(AddressSpace space);

class VarField
{
  public:
    VarField(const TVariable &var) : mVariable(&var) {}
    VarField(const TField &field) : mField(&field) {}

    ANGLE_INLINE const TVariable *variable() const { return mVariable; }

    ANGLE_INLINE const TField *field() const { return mField; }

    ANGLE_INLINE bool operator==(const VarField &other) const
    {
        return mVariable == other.mVariable && mField == other.mField;
    }

  private:
    const TVariable *mVariable = nullptr;
    const TField *mField       = nullptr;
};

}  // namespace sh

namespace std
{

template <>
struct hash<sh::VarField>
{
    size_t operator()(const sh::VarField &x) const
    {
        const sh::TVariable *var = x.variable();
        if (var)
        {
            return std::hash<const sh::TVariable *>()(var);
        }
        const sh::TField *field = x.field();
        return std::hash<const sh::TField *>()(field);
    }
};

}  // namespace std

namespace sh
{

class TemplateArg
{
  public:
    enum class Kind
    {
        Bool,
        Int,
        UInt,
        Type,
    };

    union Value
    {
        Value(bool value) : b(value) {}
        Value(int value) : i(value) {}
        Value(unsigned value) : u(value) {}
        Value(const TType &value) : t(&value) {}

        bool b;
        int i;
        unsigned u;
        const TType *t;
    };

    TemplateArg(bool value);
    TemplateArg(int value);
    TemplateArg(unsigned value);
    TemplateArg(const TType &value);

    bool operator==(const TemplateArg &other) const;
    bool operator<(const TemplateArg &other) const;

    Kind kind() const { return mKind; }
    Value value() const { return mValue; }

  public:
    Kind mKind;
    Value mValue;
};

// An environment for creating and uniquely sharing TSymbol objects.
class SymbolEnv
{
    class TemplateName
    {
        Name baseName;
        std::vector<TemplateArg> templateArgs;

      public:
        bool operator==(const TemplateName &other) const;
        bool operator<(const TemplateName &other) const;

        bool empty() const;
        void clear();

        Name fullName(std::string &buffer) const;

        void assign(const Name &name, size_t argCount, const TemplateArg *args);
    };

    using TypeRef        = CRef<TType>;
    using Sig            = std::vector<TypeRef>;  // Param0, Param1, ... ParamN, Return
    using SigToFunc      = std::map<Sig, TFunction *>;
    using Overloads      = std::map<TemplateName, SigToFunc>;
    using AngleStructs   = std::map<ImmutableString, TStructure *>;
    using TextureStructs = std::map<TBasicType, TStructure *>;

  public:
    SymbolEnv(TCompiler &compiler, TIntermBlock &root);

    TSymbolTable &symbolTable() { return mSymbolTable; }

    // There exist Angle rewrites that can lead to incoherent structs
    //
    // Example
    //    struct A { ... }
    //    struct B { A' a; } // A' has same name as A but is not identical.
    // becomes
    //    struct A { ... }
    //    struct B { A a; }
    //
    // This notably happens when A contains a sampler in the original GLSL code but is rewritten to
    // not have a sampler, yet the A' struct field still contains the sampler field.
    const TStructure &remap(const TStructure &s) const;

    // Like `TStructure &remap(const TStructure &s)` but additionally maps null to null.
    const TStructure *remap(const TStructure *s) const;

    const TFunction &getFunctionOverload(const Name &name,
                                         const TType &returnType,
                                         size_t paramCount,
                                         const TType **paramTypes,
                                         size_t templateArgCount         = 0,
                                         const TemplateArg *templateArgs = nullptr);

    TIntermAggregate &callFunctionOverload(const Name &name,
                                           const TType &returnType,
                                           TIntermSequence &args,
                                           size_t templateArgCount         = 0,
                                           const TemplateArg *templateArgs = nullptr);

    const TStructure &newStructure(const Name &name, TFieldList &fields);

    const TStructure &getTextureEnv(TBasicType samplerType);
    const TStructure &getSamplerStruct();

    void markAsPointer(VarField x, AddressSpace space);
    void removePointer(VarField x);
    const AddressSpace *isPointer(VarField x) const;

    void markAsReference(VarField x, AddressSpace space);
    void removeAsReference(VarField x);
    const AddressSpace *isReference(VarField x) const;

    void markAsPacked(const TField &field);
    bool isPacked(const TField &field) const;

    void markAsUBO(VarField x);
    bool isUBO(VarField x) const;

  private:
    const TFunction &getFunctionOverloadImpl();

    void markSpace(VarField x, AddressSpace space, std::unordered_map<VarField, AddressSpace> &map);
    void removeSpace(VarField x, std::unordered_map<VarField, AddressSpace> &map);
    const AddressSpace *isSpace(VarField x,
                                const std::unordered_map<VarField, AddressSpace> &map) const;

  private:
    TSymbolTable &mSymbolTable;

    std::map<Name, const TStructure *> mNameToStruct;
    Overloads mOverloads;
    Sig mReusableSigBuffer;
    TemplateName mReusableTemplateNameBuffer;
    std::string mReusableStringBuffer;

    AngleStructs mAngleStructs;
    std::unordered_map<TBasicType, const TStructure *> mTextureEnvs;
    const TStructure *mSampler = nullptr;

    std::unordered_map<VarField, AddressSpace> mPointers;
    std::unordered_map<VarField, AddressSpace> mReferences;
    std::unordered_set<const TField *> mPackedFields;
    std::unordered_set<VarField> mUboFields;
};

Name GetTextureTypeName(TBasicType samplerType);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_SYMBOLENV_H_
