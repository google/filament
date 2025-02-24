//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/msl/IntroduceVertexIndexID.h"
#include "compiler/translator/IntermRebuild.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/msl/AstHelpers.h"
#include "compiler/translator/tree_util/BuiltIn.h"
using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{

constexpr const TVariable kgl_VertexIDMetal(BuiltInId::gl_VertexID,
                                            ImmutableString("vertexIDMetal"),
                                            SymbolType::AngleInternal,
                                            TExtension::UNDEFINED,
                                            StaticType::Get<EbtUInt, EbpHigh, EvqVertexID, 1, 1>());

constexpr const TVariable kgl_instanceIdMetal(
    BuiltInId::gl_InstanceID,
    ImmutableString("instanceIdMod"),
    SymbolType::AngleInternal,
    TExtension::UNDEFINED,
    StaticType::Get<EbtUInt, EbpHigh, EvqInstanceID, 1, 1>());

constexpr const TVariable kgl_baseInstanceMetal(
    BuiltInId::gl_BaseInstance,
    ImmutableString("baseInstance"),
    SymbolType::AngleInternal,
    TExtension::UNDEFINED,
    StaticType::Get<EbtUInt, EbpHigh, EvqInstanceID, 1, 1>());

class Rewriter : public TIntermRebuild
{
  public:
    Rewriter(TCompiler &compiler) : TIntermRebuild(compiler, true, true) {}

  private:
    PreResult visitFunctionDefinitionPre(TIntermFunctionDefinition &node) override
    {
        if (node.getFunction()->isMain())
        {
            const TFunction *mainFunction = node.getFunction();
            bool needsVertexId            = true;
            bool needsInstanceId          = true;
            std::vector<const TVariable *> mVariablesToIntroduce;
            for (size_t i = 0; i < mainFunction->getParamCount(); ++i)
            {
                const TVariable *param = mainFunction->getParam(i);
                Name instanceIDName =
                    Pipeline{Pipeline::Type::InstanceId, nullptr}.getStructInstanceName(
                        Pipeline::Variant::Modified);
                if (Name(*param) == instanceIDName)
                {
                    needsInstanceId = false;
                }
                else if (param->getType().getQualifier() == TQualifier::EvqVertexID)
                {
                    needsVertexId = false;
                }
            }
            if (needsInstanceId)
            {
                // Ensure these variables are present because they are required for XFB emulation.
                mVariablesToIntroduce.push_back(&kgl_instanceIdMetal);
                mVariablesToIntroduce.push_back(&kgl_baseInstanceMetal);
            }
            if (needsVertexId)
            {
                mVariablesToIntroduce.push_back(&kgl_VertexIDMetal);
            }
            const TFunction &newFunction = CloneFunctionAndAppendParams(
                mSymbolTable, nullptr, *node.getFunction(), mVariablesToIntroduce);
            TIntermFunctionPrototype *newProto = new TIntermFunctionPrototype(&newFunction);
            return new TIntermFunctionDefinition(newProto, node.getBody());
        }
        return node;
    }
};

}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

bool sh::IntroduceVertexAndInstanceIndex(TCompiler &compiler, TIntermBlock &root)
{
    if (!Rewriter(compiler).rebuildRoot(root))
    {
        return false;
    }
    return true;
}
