//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Analysis of the AST needed for HLSL generation

#include "compiler/translator/hlsl/ASTMetadataHLSL.h"

#include "compiler/translator/CallDAG.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

// Class used to traverse the AST of a function definition, checking if the
// function uses a gradient, and writing the set of control flow using gradients.
// It assumes that the analysis has already been made for the function's
// callees.
class PullGradient : public TIntermTraverser
{
  public:
    PullGradient(MetadataList *metadataList, size_t index, const CallDAG &dag)
        : TIntermTraverser(true, false, true),
          mMetadataList(metadataList),
          mMetadata(&(*metadataList)[index]),
          mIndex(index),
          mDag(dag)
    {
        ASSERT(index < metadataList->size());

        // ESSL 100 builtin gradient functions
        mGradientBuiltinFunctions.insert(ImmutableString("texture2D"));
        mGradientBuiltinFunctions.insert(ImmutableString("texture2DProj"));
        mGradientBuiltinFunctions.insert(ImmutableString("textureCube"));

        // ESSL 300 builtin gradient functions
        mGradientBuiltinFunctions.insert(ImmutableString("dFdx"));
        mGradientBuiltinFunctions.insert(ImmutableString("dFdy"));
        mGradientBuiltinFunctions.insert(ImmutableString("fwidth"));
        mGradientBuiltinFunctions.insert(ImmutableString("texture"));
        mGradientBuiltinFunctions.insert(ImmutableString("textureProj"));
        mGradientBuiltinFunctions.insert(ImmutableString("textureOffset"));
        mGradientBuiltinFunctions.insert(ImmutableString("textureProjOffset"));

        // ESSL 310 doesn't add builtin gradient functions
    }

    void traverse(TIntermFunctionDefinition *node)
    {
        node->traverse(this);
        ASSERT(mParents.empty());
    }

    // Called when a gradient operation or a call to a function using a gradient is found.
    void onGradient()
    {
        mMetadata->mUsesGradient = true;
        // Mark the latest control flow as using a gradient.
        if (!mParents.empty())
        {
            mMetadata->mControlFlowsContainingGradient.insert(mParents.back());
        }
    }

    void visitControlFlow(Visit visit, TIntermNode *node)
    {
        if (visit == PreVisit)
        {
            mParents.push_back(node);
        }
        else if (visit == PostVisit)
        {
            ASSERT(mParents.back() == node);
            mParents.pop_back();
            // A control flow's using a gradient means its parents are too.
            if (mMetadata->mControlFlowsContainingGradient.count(node) > 0 && !mParents.empty())
            {
                mMetadata->mControlFlowsContainingGradient.insert(mParents.back());
            }
        }
    }

    bool visitLoop(Visit visit, TIntermLoop *loop) override
    {
        visitControlFlow(visit, loop);
        return true;
    }

    bool visitIfElse(Visit visit, TIntermIfElse *ifElse) override
    {
        visitControlFlow(visit, ifElse);
        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (visit == PreVisit)
        {
            if (node->getOp() == EOpCallFunctionInAST)
            {
                size_t calleeIndex = mDag.findIndex(node->getFunction()->uniqueId());
                ASSERT(calleeIndex != CallDAG::InvalidIndex && calleeIndex < mIndex);

                if ((*mMetadataList)[calleeIndex].mUsesGradient)
                {
                    onGradient();
                }
            }
            else if (BuiltInGroup::IsBuiltIn(node->getOp()) && !BuiltInGroup::IsMath(node->getOp()))
            {
                if (mGradientBuiltinFunctions.find(node->getFunction()->name()) !=
                    mGradientBuiltinFunctions.end())
                {
                    onGradient();
                }
            }
        }

        return true;
    }

  private:
    MetadataList *mMetadataList;
    ASTMetadataHLSL *mMetadata;
    size_t mIndex;
    const CallDAG &mDag;

    // Contains a stack of the control flow nodes that are parents of the node being
    // currently visited. It is used to mark control flows using a gradient.
    std::vector<TIntermNode *> mParents;

    // A list of builtin functions that use gradients
    std::set<ImmutableString> mGradientBuiltinFunctions;
};

// Traverses the AST of a function definition to compute the the discontinuous loops
// and the if statements containing gradient loops. It assumes that the gradient loops
// (loops that contain a gradient) have already been computed and that it has already
// traversed the current function's callees.
class PullComputeDiscontinuousAndGradientLoops : public TIntermTraverser
{
  public:
    PullComputeDiscontinuousAndGradientLoops(MetadataList *metadataList,
                                             size_t index,
                                             const CallDAG &dag)
        : TIntermTraverser(true, false, true),
          mMetadataList(metadataList),
          mMetadata(&(*metadataList)[index]),
          mIndex(index),
          mDag(dag)
    {}

    void traverse(TIntermFunctionDefinition *node)
    {
        node->traverse(this);
        ASSERT(mLoopsAndSwitches.empty());
        ASSERT(mIfs.empty());
    }

    // Called when traversing a gradient loop or a call to a function with a
    // gradient loop in its call graph.
    void onGradientLoop()
    {
        mMetadata->mHasGradientLoopInCallGraph = true;
        // Mark the latest if as using a discontinuous loop.
        if (!mIfs.empty())
        {
            mMetadata->mIfsContainingGradientLoop.insert(mIfs.back());
        }
    }

    bool visitLoop(Visit visit, TIntermLoop *loop) override
    {
        if (visit == PreVisit)
        {
            mLoopsAndSwitches.push_back(loop);

            if (mMetadata->hasGradientInCallGraph(loop))
            {
                onGradientLoop();
            }
        }
        else if (visit == PostVisit)
        {
            ASSERT(mLoopsAndSwitches.back() == loop);
            mLoopsAndSwitches.pop_back();
        }

        return true;
    }

    bool visitIfElse(Visit visit, TIntermIfElse *node) override
    {
        if (visit == PreVisit)
        {
            mIfs.push_back(node);
        }
        else if (visit == PostVisit)
        {
            ASSERT(mIfs.back() == node);
            mIfs.pop_back();
            // An if using a discontinuous loop means its parents ifs are also discontinuous.
            if (mMetadata->mIfsContainingGradientLoop.count(node) > 0 && !mIfs.empty())
            {
                mMetadata->mIfsContainingGradientLoop.insert(mIfs.back());
            }
        }

        return true;
    }

    bool visitBranch(Visit visit, TIntermBranch *node) override
    {
        if (visit == PreVisit)
        {
            switch (node->getFlowOp())
            {
                case EOpBreak:
                {
                    ASSERT(!mLoopsAndSwitches.empty());
                    TIntermLoop *loop = mLoopsAndSwitches.back()->getAsLoopNode();
                    if (loop != nullptr)
                    {
                        mMetadata->mDiscontinuousLoops.insert(loop);
                    }
                }
                break;
                case EOpContinue:
                {
                    ASSERT(!mLoopsAndSwitches.empty());
                    TIntermLoop *loop = nullptr;
                    size_t i          = mLoopsAndSwitches.size();
                    while (loop == nullptr && i > 0)
                    {
                        --i;
                        loop = mLoopsAndSwitches.at(i)->getAsLoopNode();
                    }
                    ASSERT(loop != nullptr);
                    mMetadata->mDiscontinuousLoops.insert(loop);
                }
                break;
                case EOpKill:
                case EOpReturn:
                    // A return or discard jumps out of all the enclosing loops
                    if (!mLoopsAndSwitches.empty())
                    {
                        for (TIntermNode *intermNode : mLoopsAndSwitches)
                        {
                            TIntermLoop *loop = intermNode->getAsLoopNode();
                            if (loop)
                            {
                                mMetadata->mDiscontinuousLoops.insert(loop);
                            }
                        }
                    }
                    break;
                default:
                    UNREACHABLE();
            }
        }

        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (visit == PreVisit && node->getOp() == EOpCallFunctionInAST)
        {
            size_t calleeIndex = mDag.findIndex(node->getFunction()->uniqueId());
            ASSERT(calleeIndex != CallDAG::InvalidIndex && calleeIndex < mIndex);

            if ((*mMetadataList)[calleeIndex].mHasGradientLoopInCallGraph)
            {
                onGradientLoop();
            }
        }

        return true;
    }

    bool visitSwitch(Visit visit, TIntermSwitch *node) override
    {
        if (visit == PreVisit)
        {
            mLoopsAndSwitches.push_back(node);
        }
        else if (visit == PostVisit)
        {
            ASSERT(mLoopsAndSwitches.back() == node);
            mLoopsAndSwitches.pop_back();
        }
        return true;
    }

  private:
    MetadataList *mMetadataList;
    ASTMetadataHLSL *mMetadata;
    size_t mIndex;
    const CallDAG &mDag;

    std::vector<TIntermNode *> mLoopsAndSwitches;
    std::vector<TIntermIfElse *> mIfs;
};

// Tags all the functions called in a discontinuous loop
class PushDiscontinuousLoops : public TIntermTraverser
{
  public:
    PushDiscontinuousLoops(MetadataList *metadataList, size_t index, const CallDAG &dag)
        : TIntermTraverser(true, true, true),
          mMetadataList(metadataList),
          mMetadata(&(*metadataList)[index]),
          mIndex(index),
          mDag(dag),
          mNestedDiscont(mMetadata->mCalledInDiscontinuousLoop ? 1 : 0)
    {}

    void traverse(TIntermFunctionDefinition *node)
    {
        node->traverse(this);
        ASSERT(mNestedDiscont == (mMetadata->mCalledInDiscontinuousLoop ? 1 : 0));
    }

    bool visitLoop(Visit visit, TIntermLoop *loop) override
    {
        bool isDiscontinuous = mMetadata->mDiscontinuousLoops.count(loop) > 0;

        if (visit == PreVisit && isDiscontinuous)
        {
            mNestedDiscont++;
        }
        else if (visit == PostVisit && isDiscontinuous)
        {
            mNestedDiscont--;
        }

        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        switch (node->getOp())
        {
            case EOpCallFunctionInAST:
                if (visit == PreVisit && mNestedDiscont > 0)
                {
                    size_t calleeIndex = mDag.findIndex(node->getFunction()->uniqueId());
                    ASSERT(calleeIndex != CallDAG::InvalidIndex && calleeIndex < mIndex);

                    (*mMetadataList)[calleeIndex].mCalledInDiscontinuousLoop = true;
                }
                break;
            default:
                break;
        }
        return true;
    }

  private:
    MetadataList *mMetadataList;
    ASTMetadataHLSL *mMetadata;
    size_t mIndex;
    const CallDAG &mDag;

    int mNestedDiscont;
};
}  // namespace

bool ASTMetadataHLSL::hasGradientInCallGraph(TIntermLoop *node)
{
    return mControlFlowsContainingGradient.count(node) > 0;
}

bool ASTMetadataHLSL::hasGradientLoop(TIntermIfElse *node)
{
    return mIfsContainingGradientLoop.count(node) > 0;
}

MetadataList CreateASTMetadataHLSL(TIntermNode *root, const CallDAG &callDag)
{
    MetadataList metadataList(callDag.size());

    // Compute all the information related to when gradient operations are used.
    // We want to know for each function and control flow operation if they have
    // a gradient operation in their call graph (shortened to "using a gradient"
    // in the rest of the file).
    //
    // This computation is logically split in three steps:
    //  1 - For each function compute if it uses a gradient in its body, ignoring
    // calls to other user-defined functions.
    //  2 - For each function determine if it uses a gradient in its call graph,
    // using the result of step 1 and the CallDAG to know its callees.
    //  3 - For each control flow statement of each function, check if it uses a
    // gradient in the function's body, or if it calls a user-defined function that
    // uses a gradient.
    //
    // We take advantage of the call graph being a DAG and instead compute 1, 2 and 3
    // for leaves first, then going down the tree. This is correct because 1 doesn't
    // depend on other functions, and 2 and 3 depend only on callees.
    for (size_t i = 0; i < callDag.size(); i++)
    {
        PullGradient pull(&metadataList, i, callDag);
        pull.traverse(callDag.getRecordFromIndex(i).node);
    }

    // Compute which loops are discontinuous and which function are called in
    // these loops. The same way computing gradient usage is a "pull" process,
    // computing "bing used in a discont. loop" is a push process. However we also
    // need to know what ifs have a discontinuous loop inside so we do the same type
    // of callgraph analysis as for the gradient.

    // First compute which loops are discontinuous (no specific order) and pull
    // the ifs and functions using a gradient loop.
    for (size_t i = 0; i < callDag.size(); i++)
    {
        PullComputeDiscontinuousAndGradientLoops pull(&metadataList, i, callDag);
        pull.traverse(callDag.getRecordFromIndex(i).node);
    }

    // Then push the information to callees, either from the a local discontinuous
    // loop or from the caller being called in a discontinuous loop already
    for (size_t i = callDag.size(); i-- > 0;)
    {
        PushDiscontinuousLoops push(&metadataList, i, callDag);
        push.traverse(callDag.getRecordFromIndex(i).node);
    }

    // We create "Lod0" version of functions with the gradient operations replaced
    // by non-gradient operations so that the D3D compiler is happier with discont
    // loops.
    for (auto &metadata : metadataList)
    {
        metadata.mNeedsLod0 = metadata.mCalledInDiscontinuousLoop && metadata.mUsesGradient;
    }

    return metadataList;
}

}  // namespace sh
