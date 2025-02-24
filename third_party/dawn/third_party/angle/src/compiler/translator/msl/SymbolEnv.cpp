//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <algorithm>
#include <limits>

#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/IntermRebuild.h"
#include "compiler/translator/msl/AstHelpers.h"
#include "compiler/translator/msl/SymbolEnv.h"
#include "compiler/translator/util.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

constexpr AddressSpace kAddressSpaces[] = {
    AddressSpace::Constant,
    AddressSpace::Device,
    AddressSpace::Thread,
};

char const *sh::toString(AddressSpace space)
{
    switch (space)
    {
        case AddressSpace::Constant:
            return "constant";
        case AddressSpace::Device:
            return "device";
        case AddressSpace::Thread:
            return "thread";
    }
}

////////////////////////////////////////////////////////////////////////////////

using NameToStruct = std::map<Name, const TStructure *>;

class StructFinder : TIntermRebuild
{
    NameToStruct nameToStruct;

    StructFinder(TCompiler &compiler) : TIntermRebuild(compiler, true, false) {}

    PreResult visitDeclarationPre(TIntermDeclaration &node) override
    {
        Declaration decl     = ViewDeclaration(node);
        const TVariable &var = decl.symbol.variable();
        const TType &type    = var.getType();

        if (var.symbolType() == SymbolType::Empty && type.isStructSpecifier())
        {
            const TStructure *s = type.getStruct();
            ASSERT(s);
            const Name name(*s);
            const TStructure *&z = nameToStruct[name];
            ASSERT(!z);
            z = s;
        }

        return node;
    }

    PreResult visitFunctionDefinitionPre(TIntermFunctionDefinition &node) override
    {
        return {node, VisitBits::Neither};
    }

  public:
    static NameToStruct FindStructs(TCompiler &compiler, TIntermBlock &root)
    {
        StructFinder finder(compiler);
        if (!finder.rebuildRoot(root))
        {
            UNREACHABLE();
        }
        return std::move(finder.nameToStruct);
    }
};

////////////////////////////////////////////////////////////////////////////////

TemplateArg::TemplateArg(bool value) : mKind(Kind::Bool), mValue(value) {}

TemplateArg::TemplateArg(int value) : mKind(Kind::Int), mValue(value) {}

TemplateArg::TemplateArg(unsigned value) : mKind(Kind::UInt), mValue(value) {}

TemplateArg::TemplateArg(const TType &value) : mKind(Kind::Type), mValue(value) {}

bool TemplateArg::operator==(const TemplateArg &other) const
{
    if (mKind != other.mKind)
    {
        return false;
    }

    switch (mKind)
    {
        case Kind::Bool:
            return mValue.b == other.mValue.b;
        case Kind::Int:
            return mValue.i == other.mValue.i;
        case Kind::UInt:
            return mValue.u == other.mValue.u;
        case Kind::Type:
            return *mValue.t == *other.mValue.t;
    }
}

bool TemplateArg::operator<(const TemplateArg &other) const
{
    if (mKind < other.mKind)
    {
        return true;
    }

    if (mKind > other.mKind)
    {
        return false;
    }

    switch (mKind)
    {
        case Kind::Bool:
            return mValue.b < other.mValue.b;
        case Kind::Int:
            return mValue.i < other.mValue.i;
        case Kind::UInt:
            return mValue.u < other.mValue.u;
        case Kind::Type:
            return *mValue.t < *other.mValue.t;
    }
}

////////////////////////////////////////////////////////////////////////////////

bool SymbolEnv::TemplateName::operator==(const TemplateName &other) const
{
    return baseName == other.baseName && templateArgs == other.templateArgs;
}

bool SymbolEnv::TemplateName::operator<(const TemplateName &other) const
{
    if (baseName < other.baseName)
    {
        return true;
    }
    if (other.baseName < baseName)
    {
        return false;
    }
    return templateArgs < other.templateArgs;
}

bool SymbolEnv::TemplateName::empty() const
{
    return baseName.empty() && templateArgs.empty();
}

void SymbolEnv::TemplateName::clear()
{
    baseName = Name();
    templateArgs.clear();
}

Name SymbolEnv::TemplateName::fullName(std::string &buffer) const
{
    ASSERT(buffer.empty());

    if (templateArgs.empty())
    {
        return baseName;
    }

    static constexpr size_t n = std::max({
        std::numeric_limits<unsigned>::digits10,  //
        std::numeric_limits<int>::digits10,       //
        5,                                        // max_length("true", "false")
    });

    buffer.reserve(baseName.rawName().length() + (n + 2) * templateArgs.size() + 1);
    buffer += baseName.rawName().data();

    if (!templateArgs.empty())
    {
        buffer += "<";

        bool first = true;
        char argBuffer[n + 1];
        for (const TemplateArg &arg : templateArgs)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                buffer += ", ";
            }

            const TemplateArg::Value value = arg.value();
            const TemplateArg::Kind kind   = arg.kind();
            switch (kind)
            {
                case TemplateArg::Kind::Bool:
                    if (value.b)
                    {
                        buffer += "true";
                    }
                    else
                    {
                        buffer += "false";
                    }
                    break;

                case TemplateArg::Kind::Int:
                    snprintf(argBuffer, sizeof(argBuffer), "%i", value.i);
                    buffer += argBuffer;
                    break;

                case TemplateArg::Kind::UInt:
                    snprintf(argBuffer, sizeof(argBuffer), "%u", value.u);
                    buffer += argBuffer;
                    break;

                case TemplateArg::Kind::Type:
                {
                    const TType &type = *value.t;
                    if (const TStructure *s = type.getStruct())
                    {
                        buffer += s->name().data();
                    }
                    else if (HasScalarBasicType(type))
                    {
                        ASSERT(!type.isArray());  // TODO
                        buffer += type.getBasicString();
                        if (type.isVector())
                        {
                            snprintf(argBuffer, sizeof(argBuffer), "%u", type.getNominalSize());
                            buffer += argBuffer;
                        }
                        else if (type.isMatrix())
                        {
                            snprintf(argBuffer, sizeof(argBuffer), "%u", type.getCols());
                            buffer += argBuffer;
                            buffer += "x";
                            snprintf(argBuffer, sizeof(argBuffer), "%u", type.getRows());
                            buffer += argBuffer;
                        }
                    }
                }
                break;
            }
        }

        buffer += ">";
    }

    const ImmutableString name(buffer);
    buffer.clear();

    return Name(name, baseName.symbolType());
}

void SymbolEnv::TemplateName::assign(const Name &name, size_t argCount, const TemplateArg *args)
{
    baseName = name;
    templateArgs.clear();
    for (size_t i = 0; i < argCount; ++i)
    {
        templateArgs.push_back(args[i]);
    }
}

////////////////////////////////////////////////////////////////////////////////

SymbolEnv::SymbolEnv(TCompiler &compiler, TIntermBlock &root)
    : mSymbolTable(compiler.getSymbolTable()),
      mNameToStruct(StructFinder::FindStructs(compiler, root))
{}

const TStructure &SymbolEnv::remap(const TStructure &s) const
{
    const Name name(s);
    auto iter = mNameToStruct.find(name);
    if (iter == mNameToStruct.end())
    {
        return s;
    }
    const TStructure &z = *iter->second;
    return z;
}

const TStructure *SymbolEnv::remap(const TStructure *s) const
{
    if (s)
    {
        return &remap(*s);
    }
    return nullptr;
}

const TFunction &SymbolEnv::getFunctionOverloadImpl()
{
    ASSERT(!mReusableSigBuffer.empty());

    SigToFunc &sigToFunc = mOverloads[mReusableTemplateNameBuffer];
    TFunction *&func     = sigToFunc[mReusableSigBuffer];

    if (!func)
    {
        const TType &returnType = mReusableSigBuffer.back();
        mReusableSigBuffer.pop_back();

        const Name name = mReusableTemplateNameBuffer.fullName(mReusableStringBuffer);

        func = new TFunction(&mSymbolTable, name.rawName(), name.symbolType(), &returnType, false);
        for (const TType &paramType : mReusableSigBuffer)
        {
            func->addParameter(
                new TVariable(&mSymbolTable, kEmptyImmutableString, &paramType, SymbolType::Empty));
        }
    }

    mReusableSigBuffer.clear();
    mReusableTemplateNameBuffer.clear();

    return *func;
}

const TFunction &SymbolEnv::getFunctionOverload(const Name &name,
                                                const TType &returnType,
                                                size_t paramCount,
                                                const TType **paramTypes,
                                                size_t templateArgCount,
                                                const TemplateArg *templateArgs)
{
    ASSERT(mReusableSigBuffer.empty());
    ASSERT(mReusableTemplateNameBuffer.empty());

    for (size_t i = 0; i < paramCount; ++i)
    {
        mReusableSigBuffer.push_back(*paramTypes[i]);
    }
    mReusableSigBuffer.push_back(returnType);
    mReusableTemplateNameBuffer.assign(name, templateArgCount, templateArgs);
    return getFunctionOverloadImpl();
}

TIntermAggregate &SymbolEnv::callFunctionOverload(const Name &name,
                                                  const TType &returnType,
                                                  TIntermSequence &args,
                                                  size_t templateArgCount,
                                                  const TemplateArg *templateArgs)
{
    ASSERT(mReusableSigBuffer.empty());
    ASSERT(mReusableTemplateNameBuffer.empty());

    for (TIntermNode *arg : args)
    {
        TIntermTyped *targ = arg->getAsTyped();
        ASSERT(targ);
        mReusableSigBuffer.push_back(targ->getType());
    }
    mReusableSigBuffer.push_back(returnType);
    mReusableTemplateNameBuffer.assign(name, templateArgCount, templateArgs);
    const TFunction &func = getFunctionOverloadImpl();
    return *TIntermAggregate::CreateRawFunctionCall(func, &args);
}

const TStructure &SymbolEnv::newStructure(const Name &name, TFieldList &fields)
{
    ASSERT(name.symbolType() == SymbolType::AngleInternal);

    TStructure *&s = mAngleStructs[name.rawName()];
    ASSERT(!s);
    s = new TStructure(&mSymbolTable, name.rawName(), &fields, name.symbolType());
    return *s;
}

const TStructure &SymbolEnv::getTextureEnv(TBasicType samplerType)
{
    ASSERT(IsSampler(samplerType));
    const TStructure *&env = mTextureEnvs[samplerType];
    if (env == nullptr)
    {
        auto *textureType = new TType(samplerType);
        auto *texture =
            new TField(textureType, ImmutableString("texture"), kNoSourceLoc, SymbolType::BuiltIn);
        markAsPointer(*texture, AddressSpace::Thread);

        auto *sampler = new TField(new TType(&getSamplerStruct(), false),
                                   ImmutableString("sampler"), kNoSourceLoc, SymbolType::BuiltIn);
        markAsPointer(*sampler, AddressSpace::Thread);

        std::string envName;
        envName += "TextureEnv<";
        envName += GetTextureTypeName(samplerType).rawName().data();
        envName += ">";

        env = &newStructure(Name(envName, SymbolType::AngleInternal),
                            *new TFieldList{texture, sampler});
    }
    return *env;
}

const TStructure &SymbolEnv::getSamplerStruct()
{
    if (!mSampler)
    {
        mSampler = new TStructure(&mSymbolTable, ImmutableString("metal::sampler"),
                                  new TFieldList(), SymbolType::BuiltIn);
    }
    return *mSampler;
}

void SymbolEnv::markSpace(VarField x,
                          AddressSpace space,
                          std::unordered_map<VarField, AddressSpace> &map)
{
    // It is in principle permissible to have references to pointers or multiple pointers, but this
    // is not required for now and would require code changes to get right.
    ASSERT(!isPointer(x));
    ASSERT(!isReference(x));

    map[x] = space;
}

void SymbolEnv::removeSpace(VarField x, std::unordered_map<VarField, AddressSpace> &map)
{
    // It is in principle permissible to have references to pointers or multiple pointers, but this
    // is not required for now and would require code changes to get right.
    map.erase(x);
}

const AddressSpace *SymbolEnv::isSpace(VarField x,
                                       const std::unordered_map<VarField, AddressSpace> &map) const
{
    const auto iter = map.find(x);
    if (iter == map.end())
    {
        return nullptr;
    }
    const AddressSpace space = iter->second;
    const auto index         = static_cast<std::underlying_type_t<AddressSpace>>(space);
    return &kAddressSpaces[index];
}

void SymbolEnv::markAsPointer(VarField x, AddressSpace space)
{
    return markSpace(x, space, mPointers);
}

void SymbolEnv::removePointer(VarField x)
{
    return removeSpace(x, mPointers);
}

void SymbolEnv::markAsReference(VarField x, AddressSpace space)
{
    return markSpace(x, space, mReferences);
}

const AddressSpace *SymbolEnv::isPointer(VarField x) const
{
    return isSpace(x, mPointers);
}

const AddressSpace *SymbolEnv::isReference(VarField x) const
{
    return isSpace(x, mReferences);
}

void SymbolEnv::markAsPacked(const TField &field)
{
    mPackedFields.insert(&field);
}

bool SymbolEnv::isPacked(const TField &field) const
{
    return mPackedFields.find(&field) != mPackedFields.end();
}

void SymbolEnv::markAsUBO(VarField x)
{
    mUboFields.insert(x);
}

bool SymbolEnv::isUBO(VarField x) const
{
    return mUboFields.find(x) != mUboFields.end();
}

static TBasicType GetTextureBasicType(TBasicType basicType)
{
    ASSERT(IsSampler(basicType));

    switch (basicType)
    {
        case EbtSampler2D:
        case EbtSampler3D:
        case EbtSamplerCube:
        case EbtSampler2DArray:
        case EbtSamplerExternalOES:
        case EbtSamplerExternal2DY2YEXT:
        case EbtSampler2DRect:
        case EbtSampler2DMS:
        case EbtSampler2DMSArray:
        case EbtSamplerVideoWEBGL:
        case EbtSampler2DShadow:
        case EbtSamplerCubeShadow:
        case EbtSampler2DArrayShadow:
        case EbtSamplerBuffer:
        case EbtSamplerCubeArray:
        case EbtSamplerCubeArrayShadow:
        case EbtSampler2DRectShadow:
            return TBasicType::EbtFloat;

        case EbtISampler2D:
        case EbtISampler3D:
        case EbtISamplerCube:
        case EbtISampler2DArray:
        case EbtISampler2DMS:
        case EbtISampler2DMSArray:
        case EbtISampler2DRect:
        case EbtISamplerBuffer:
        case EbtISamplerCubeArray:
            return TBasicType::EbtInt;

        case EbtUSampler2D:
        case EbtUSampler3D:
        case EbtUSamplerCube:
        case EbtUSampler2DArray:
        case EbtUSampler2DMS:
        case EbtUSampler2DMSArray:
        case EbtUSampler2DRect:
        case EbtUSamplerBuffer:
        case EbtUSamplerCubeArray:
            return TBasicType::EbtUInt;

        default:
            UNREACHABLE();
            return TBasicType::EbtVoid;
    }
}

Name sh::GetTextureTypeName(TBasicType samplerType)
{
    ASSERT(IsSampler(samplerType));

    const TBasicType textureType = GetTextureBasicType(samplerType);
    const char *name;

#define HANDLE_TEXTURE_NAME(baseName)                   \
    do                                                  \
    {                                                   \
        switch (textureType)                            \
        {                                               \
            case TBasicType::EbtFloat:                  \
                name = "metal::" baseName "<float>";    \
                break;                                  \
            case TBasicType::EbtInt:                    \
                name = "metal::" baseName "<int>";      \
                break;                                  \
            case TBasicType::EbtUInt:                   \
                name = "metal::" baseName "<uint32_t>"; \
                break;                                  \
            default:                                    \
                UNREACHABLE();                          \
                name = nullptr;                         \
                break;                                  \
        }                                               \
    } while (false)

    switch (samplerType)
    {
        // Buffer textures
        case EbtSamplerBuffer:
        case EbtISamplerBuffer:
        case EbtUSamplerBuffer:
            HANDLE_TEXTURE_NAME("texture_buffer");
            break;

        // 2d textures
        case EbtSampler2D:
        case EbtISampler2D:
        case EbtUSampler2D:
        case EbtSampler2DRect:
        case EbtUSampler2DRect:
        case EbtISampler2DRect:
            HANDLE_TEXTURE_NAME("texture2d");
            break;

        // 3d textures
        case EbtSampler3D:
        case EbtISampler3D:
        case EbtUSampler3D:
            HANDLE_TEXTURE_NAME("texture3d");
            break;

        // Cube textures
        case EbtSamplerCube:
        case EbtISamplerCube:
        case EbtUSamplerCube:
            HANDLE_TEXTURE_NAME("texturecube");
            break;

        // 2d array textures
        case EbtSampler2DArray:
        case EbtUSampler2DArray:
        case EbtISampler2DArray:
            HANDLE_TEXTURE_NAME("texture2d_array");
            break;

        case EbtSampler2DMS:
        case EbtISampler2DMS:
        case EbtUSampler2DMS:
            HANDLE_TEXTURE_NAME("texture2d_ms");
            break;

        case EbtSampler2DMSArray:
        case EbtISampler2DMSArray:
        case EbtUSampler2DMSArray:
            HANDLE_TEXTURE_NAME("texture2d_ms_array");
            break;

        // cube array
        case EbtSamplerCubeArray:
        case EbtISamplerCubeArray:
        case EbtUSamplerCubeArray:
            HANDLE_TEXTURE_NAME("texturecube_array");
            break;

        // Shadow
        case EbtSampler2DRectShadow:
        case EbtSampler2DShadow:
            HANDLE_TEXTURE_NAME("depth2d");
            break;

        case EbtSamplerCubeShadow:
            HANDLE_TEXTURE_NAME("depthcube");
            break;

        case EbtSampler2DArrayShadow:
            HANDLE_TEXTURE_NAME("depth2d_array");
            break;

        case EbtSamplerCubeArrayShadow:
            HANDLE_TEXTURE_NAME("depthcube_array");
            break;

        // Extentions
        case EbtSamplerExternalOES:       // Only valid if OES_EGL_image_external exists:
        case EbtSamplerExternal2DY2YEXT:  // Only valid if GL_EXT_YUV_target exists:
        case EbtSamplerVideoWEBGL:
            UNIMPLEMENTED();
            HANDLE_TEXTURE_NAME("TODO");
            break;

        default:
            UNREACHABLE();
            name = nullptr;
            break;
    }

#undef HANDLE_TEXTURE_NAME

    return Name(name, SymbolType::BuiltIn);
}
