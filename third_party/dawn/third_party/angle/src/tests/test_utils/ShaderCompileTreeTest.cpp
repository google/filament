//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderCompileTreeTest.cpp:
//   Test that shader validation results in the correct compile status.
//

#include "tests/test_utils/ShaderCompileTreeTest.h"

#include "compiler/translator/glsl/TranslatorESSL.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

// Checks that the node traversed is a zero node. It can be made out of multiple constructors and
// constant union nodes as long as there's no arithmetic involved and all constants are zero.
class OnlyContainsZeroConstantsTraverser final : public TIntermTraverser
{
  public:
    OnlyContainsZeroConstantsTraverser()
        : TIntermTraverser(true, false, false), mOnlyContainsConstantZeros(true)
    {}

    bool visitUnary(Visit, TIntermUnary *node) override
    {
        mOnlyContainsConstantZeros = false;
        return false;
    }

    bool visitBinary(Visit, TIntermBinary *node) override
    {
        mOnlyContainsConstantZeros = false;
        return false;
    }

    bool visitTernary(Visit, TIntermTernary *node) override
    {
        mOnlyContainsConstantZeros = false;
        return false;
    }

    bool visitSwizzle(Visit, TIntermSwizzle *node) override
    {
        mOnlyContainsConstantZeros = false;
        return false;
    }

    bool visitAggregate(Visit, TIntermAggregate *node) override
    {
        if (node->getOp() != EOpConstruct)
        {
            mOnlyContainsConstantZeros = false;
            return false;
        }
        return true;
    }

    void visitSymbol(TIntermSymbol *node) override { mOnlyContainsConstantZeros = false; }

    void visitConstantUnion(TIntermConstantUnion *node) override
    {
        if (!mOnlyContainsConstantZeros)
        {
            return;
        }

        const TType &type = node->getType();
        size_t objectSize = type.getObjectSize();
        for (size_t i = 0u; i < objectSize && mOnlyContainsConstantZeros; ++i)
        {
            bool isZero = false;
            switch (type.getBasicType())
            {
                case EbtFloat:
                    isZero = (node->getFConst(i) == 0.0f);
                    break;
                case EbtInt:
                    isZero = (node->getIConst(i) == 0);
                    break;
                case EbtUInt:
                    isZero = (node->getUConst(i) == 0u);
                    break;
                case EbtBool:
                    isZero = (node->getBConst(i) == false);
                    break;
                default:
                    // Cannot handle.
                    break;
            }
            if (!isZero)
            {
                mOnlyContainsConstantZeros = false;
                return;
            }
        }
    }

    bool onlyContainsConstantZeros() const { return mOnlyContainsConstantZeros; }

  private:
    bool mOnlyContainsConstantZeros;
};

}  // anonymous namespace

void ShaderCompileTreeTest::SetUp()
{
    mAllocator.push();
    SetGlobalPoolAllocator(&mAllocator);

    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);

    initResources(&resources);

    mTranslator = new TranslatorESSL(getShaderType(), getShaderSpec());
    ASSERT_TRUE(mTranslator->Init(resources));
}

void ShaderCompileTreeTest::TearDown()
{
    delete mTranslator;

    SetGlobalPoolAllocator(nullptr);
    mAllocator.pop();
}

bool ShaderCompileTreeTest::compile(const std::string &shaderString)
{
    const char *shaderStrings[] = {shaderString.c_str()};
    mASTRoot            = mTranslator->compileTreeForTesting(shaderStrings, 1, mCompileOptions);
    TInfoSink &infoSink = mTranslator->getInfoSink();
    mInfoLog            = infoSink.info.c_str();
    return mASTRoot != nullptr;
}

void ShaderCompileTreeTest::compileAssumeSuccess(const std::string &shaderString)
{
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation into ESSL failed, log:\n" << mInfoLog;
    }
}

bool ShaderCompileTreeTest::hasWarning() const
{
    return mInfoLog.find("WARNING: ") != std::string::npos;
}

const std::vector<sh::ShaderVariable> &ShaderCompileTreeTest::getUniforms() const
{
    return mTranslator->getUniforms();
}

const std::vector<sh::ShaderVariable> &ShaderCompileTreeTest::getAttributes() const
{
    return mTranslator->getAttributes();
}

bool IsZero(TIntermNode *node)
{
    if (!node->getAsTyped())
    {
        return false;
    }
    OnlyContainsZeroConstantsTraverser traverser;
    node->traverse(&traverser);
    return traverser.onlyContainsConstantZeros();
}

}  // namespace sh
