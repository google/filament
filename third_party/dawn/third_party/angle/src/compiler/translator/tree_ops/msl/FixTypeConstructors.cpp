
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include "compiler/translator/tree_ops/msl/FixTypeConstructors.h"
#include <unordered_map>
#include "compiler/translator/IntermRebuild.h"
#include "compiler/translator/msl/AstHelpers.h"

using namespace sh;
////////////////////////////////////////////////////////////////////////////////
namespace
{
class FixTypeTraverser : public TIntermTraverser
{
  public:
    FixTypeTraverser() : TIntermTraverser(false, false, true) {}

    bool visitAggregate(Visit visit, TIntermAggregate *aggregateNode) override
    {
        if (visit != Visit::PostVisit)
        {
            return true;
        }
        if (aggregateNode->isConstructor())
        {
            const TType &retType = aggregateNode->getType();
            if (retType.isScalar())
            {
                // No-op.
            }
            else if (retType.isVector())
            {
                size_t primarySize    = retType.getNominalSize() * retType.getArraySizeProduct();
                TIntermSequence *args = aggregateNode->getSequence();
                size_t argsSize       = 0;
                size_t beforeSize     = 0;
                TIntermNode *lastArg  = nullptr;
                for (TIntermNode *&arg : *args)
                {
                    TIntermTyped *targ = arg->getAsTyped();
                    lastArg            = arg;
                    if (targ)
                    {
                        argsSize += targ->getNominalSize();
                    }
                    if (argsSize <= primarySize)
                    {
                        beforeSize += targ->getNominalSize();
                    }
                }
                if (argsSize > primarySize)
                {
                    size_t swizzleSize         = primarySize - beforeSize;
                    TIntermTyped *targ         = lastArg->getAsTyped();
                    TIntermSwizzle *newSwizzle = nullptr;
                    switch (swizzleSize)
                    {
                        case 1:
                            newSwizzle = new TIntermSwizzle(targ->deepCopy(), {0});
                            break;
                        case 2:
                            newSwizzle = new TIntermSwizzle(targ->deepCopy(), {0, 1});
                            break;
                        case 3:
                            newSwizzle = new TIntermSwizzle(targ->deepCopy(), {0, 1, 2});
                            break;
                        default:
                            UNREACHABLE();  // Should not be reached in case of 0, or 4
                    }
                    if (newSwizzle)
                    {
                        this->queueReplacementWithParent(aggregateNode, lastArg, newSwizzle,
                                                         OriginalNode::IS_DROPPED);
                    }
                }
            }
            else if (retType.isMatrix())
            {
                // TBD if issues
            }
        }
        return true;
    }
};

}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

bool sh::FixTypeConstructors(TCompiler &compiler, SymbolEnv &symbolEnv, TIntermBlock &root)
{
    FixTypeTraverser traverser;
    root.traverse(&traverser);
    if (!traverser.updateTree(&compiler, &root))
    {
        return false;
    }
    return true;
}
