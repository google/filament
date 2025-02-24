//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/msl/RewriteOutArgs.h"
#include "compiler/translator/IntermRebuild.h"

using namespace sh;

namespace
{

template <typename T>
class SmallMultiSet
{
  public:
    struct Entry
    {
        T elem;
        size_t count;
    };

    const Entry *find(const T &x) const
    {
        for (auto &entry : mEntries)
        {
            if (x == entry.elem)
            {
                return &entry;
            }
        }
        return nullptr;
    }

    size_t multiplicity(const T &x) const
    {
        const Entry *entry = find(x);
        return entry ? entry->count : 0;
    }

    const Entry &insert(const T &x)
    {
        Entry *entry = findMutable(x);
        if (entry)
        {
            ++entry->count;
            return *entry;
        }
        else
        {
            mEntries.push_back({x, 1});
            return mEntries.back();
        }
    }

    void clear() { mEntries.clear(); }

    bool empty() const { return mEntries.empty(); }

    size_t uniqueSize() const { return mEntries.size(); }

  private:
    ANGLE_INLINE Entry *findMutable(const T &x) { return const_cast<Entry *>(find(x)); }

  private:
    std::vector<Entry> mEntries;
};

const TVariable *GetVariable(TIntermNode &node)
{
    TIntermTyped *tyNode = node.getAsTyped();
    ASSERT(tyNode);
    if (TIntermSymbol *symbol = tyNode->getAsSymbolNode())
    {
        return &symbol->variable();
    }
    return nullptr;
}

class Rewriter : public TIntermRebuild
{
    SmallMultiSet<const TVariable *> mVarBuffer;  // reusable buffer
    SymbolEnv &mSymbolEnv;

  public:
    ~Rewriter() override { ASSERT(mVarBuffer.empty()); }

    Rewriter(TCompiler &compiler, SymbolEnv &symbolEnv)
        : TIntermRebuild(compiler, false, true), mSymbolEnv(symbolEnv)
    {}

    static bool argAlreadyProcessed(TIntermTyped *arg)
    {
        if (arg->getAsAggregate())
        {
            const TFunction *func = arg->getAsAggregate()->getFunction();
            // These two builtins already generate references, and the
            // ANGLE_inout and ANGLE_out overloads in ProgramPrelude are both
            // unnecessary and incompatible.
            if (func && func->symbolType() == SymbolType::AngleInternal &&
                (func->name() == "swizzle_ref" || func->name() == "elem_ref"))
            {
                return true;
            }
        }
        return false;
    }

    PostResult visitAggregatePost(TIntermAggregate &aggregateNode) override
    {
        ASSERT(mVarBuffer.empty());

        const TFunction *func = aggregateNode.getFunction();
        if (!func)
        {
            return aggregateNode;
        }

        TIntermSequence &args = *aggregateNode.getSequence();
        size_t argCount       = args.size();

        auto getParamQualifier = [&](size_t i) {
            const TVariable &param     = *func->getParam(i);
            const TType &paramType     = param.getType();
            const TQualifier paramQual = paramType.getQualifier();
            return paramQual;
        };

        // Check which params might be aliased, and mark all out params as references.
        bool mightAlias = false;
        for (size_t i = 0; i < argCount; ++i)
        {
            const TQualifier paramQual = getParamQualifier(i);

            switch (paramQual)
            {
                case TQualifier::EvqParamOut:
                case TQualifier::EvqParamInOut:
                {
                    const TVariable &param = *func->getParam(i);
                    if (!mSymbolEnv.isReference(param))
                    {
                        mSymbolEnv.markAsReference(param, AddressSpace::Thread);
                    }
                    // Note: not the same as param above, this refers to the variable in the
                    // argument list in the callsite.
                    const TVariable *var = GetVariable(*args[i]);
                    if (mVarBuffer.insert(var).count > 1)
                    {
                        mightAlias = true;
                    }
                }
                break;

                default:
                {
                    // If a function directly accesses global or stage output variables, the
                    // relevant internal struct is passed in as a parameter during translation.
                    // Ensure it is included in the aliasing checks.
                    const TVariable *var = GetVariable(*args[i]);
                    if (var != nullptr && var->symbolType() == SymbolType::AngleInternal)
                    {
                        // These names are set in Pipeline::getStructInstanceName
                        const ImmutableString &name = var->name();
                        if (name == "vertexOut" || name == "fragmentOut" ||
                            name == "nonConstGlobals")
                        {
                            mVarBuffer.insert(var);
                        }
                    }
                }
                break;
            }
        }

        // Non-symbol (e.g., TIntermBinary) parameters are cached as null pointers.
        const bool hasIndeterminateVar = mVarBuffer.find(nullptr);

        if (!mightAlias)
        {
            // Support aliasing when there is only one unresolved parameter
            // and at least one resolved parameter. This may happen in the
            // following cases:
            //
            //   - A struct member (or an array element) is passed along with the struct
            //
            //         struct S { float f; };
            //         void foo(out S a, out float b) {...}
            //         void bar() {
            //             S s;
            //             foo(s, s.f);
            //         }
            //
            //     mVarBuffer: s and nullptr (for s.f)
            //
            //   - A global (or built-in) variable is passed as an out/inout
            //     parameter and also used in the called function directly
            //
            //         float x;
            //         bool foo(out float a) {
            //             a = 2.0;
            //             return x == 1.0 && a == 2.0;
            //         }
            //         void bar() {
            //             x = 1.0;
            //             foo(x); // == true
            //         }
            //
            //     In this case, foo and x will be translated to
            //
            //         struct ANGLE_NonConstGlobals { float _ux; };
            //         bool _ufoo(thread ANGLE_NonConstGlobals & ANGLE_nonConstGlobals,
            //                    thread float & _ua)
            //
            //     mVarBuffer: nonConstGlobals and nullptr (for nonConstGlobals._ux)
            mightAlias = hasIndeterminateVar && mVarBuffer.uniqueSize() > 1;
        }

        if (mightAlias)
        {
            for (size_t i = 0; i < argCount; ++i)
            {
                TIntermTyped *arg = args[i]->getAsTyped();
                ASSERT(arg);
                if (!argAlreadyProcessed(arg))
                {
                    const TVariable *var       = GetVariable(*arg);
                    const TQualifier paramQual = getParamQualifier(i);

                    if (hasIndeterminateVar || mVarBuffer.multiplicity(var) > 1)
                    {
                        switch (paramQual)
                        {
                            case TQualifier::EvqParamOut:
                                args[i] = &mSymbolEnv.callFunctionOverload(
                                    Name("out"), arg->getType(), *new TIntermSequence{arg});
                                break;

                            case TQualifier::EvqParamInOut:
                                args[i] = &mSymbolEnv.callFunctionOverload(
                                    Name("inout"), arg->getType(), *new TIntermSequence{arg});
                                break;

                            default:
                                break;
                        }
                    }
                }
            }
        }

        mVarBuffer.clear();

        return aggregateNode;
    }
};

}  // anonymous namespace

bool sh::RewriteOutArgs(TCompiler &compiler, TIntermBlock &root, SymbolEnv &symbolEnv)
{
    Rewriter rewriter(compiler, symbolEnv);
    if (!rewriter.rebuildRoot(root))
    {
        return false;
    }
    return true;
}
