//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/msl/AstHelpers.h"
#include "compiler/translator/msl/ToposortStructs.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{

template <typename T>
using Edges = std::unordered_set<T>;

template <typename T>
using Graph = std::unordered_map<T, Edges<T>>;

struct EdgeComparator
{
    bool operator()(const TStructure *s1, const TStructure *s2) { return s2->name() < s1->name(); }
};

void BuildGraphImpl(SymbolEnv &symbolEnv, Graph<const TStructure *> &g, const TStructure *s)
{
    if (g.find(s) != g.end())
    {
        return;
    }

    Edges<const TStructure *> &es = g[s];

    const TFieldList &fs = s->fields();
    for (const TField *f : fs)
    {
        if (const TStructure *z = symbolEnv.remap(f->type()->getStruct()))
        {
            es.insert(z);
            BuildGraphImpl(symbolEnv, g, z);
            Edges<const TStructure *> &ez = g[z];
            es.insert(ez.begin(), ez.end());
        }
    }
}

Graph<const TStructure *> BuildGraph(SymbolEnv &symbolEnv,
                                     const std::vector<const TStructure *> &structs)
{
    Graph<const TStructure *> g;
    for (const TStructure *s : structs)
    {
        BuildGraphImpl(symbolEnv, g, s);
    }
    return g;
}

std::vector<const TStructure *> SortEdges(const std::unordered_set<const TStructure *> &structs)
{
    std::vector<const TStructure *> sorted;
    sorted.reserve(structs.size());
    sorted.insert(sorted.begin(), structs.begin(), structs.end());
    std::sort(sorted.begin(), sorted.end(), EdgeComparator());
    return sorted;
}

// Algorthm: https://en.wikipedia.org/wiki/Topological_sorting#Depth-first_search
// Note that the algorithm is modified to visit nodes in sorted order. This
// ensures consistent results. Without this, the returned order (in so far as
// leaf nodes) is undefined, because iterating over an unordered_set of pointers
// depends upon the actual pointer values. Consistent results is important for
// code that keys off the string of shaders for caching.
template <typename T>
std::vector<T> Toposort(const Graph<T> &g)
{
    // nodes with temporary mark
    std::unordered_set<T> temps;

    // nodes without permanent mark
    std::unordered_set<T> invPerms;
    for (const auto &entry : g)
    {
        invPerms.insert(entry.first);
    }

    // L <- Empty list that will contain the sorted elements
    std::vector<T> L;

    // function visit(node n)
    std::function<void(T)> visit = [&](T n) -> void {
        // if n has a permanent mark then
        if (invPerms.find(n) == invPerms.end())
        {
            // return
            return;
        }
        // if n has a temporary mark then
        if (temps.find(n) != temps.end())
        {
            // stop   (not a DAG)
            UNREACHABLE();
        }

        // mark n with a temporary mark
        temps.insert(n);

        // for each node m with an edge from n to m do
        auto enIter = g.find(n);
        ASSERT(enIter != g.end());

        std::vector<T> sorted = SortEdges(enIter->second);
        for (T m : sorted)
        {
            // visit(m)
            visit(m);
        }

        // remove temporary mark from n
        temps.erase(n);
        // mark n with a permanent mark
        invPerms.erase(n);
        // add n to head of L
        L.push_back(n);
    };

    // while exists nodes without a permanent mark do
    while (!invPerms.empty())
    {
        // select an unmarked node n
        std::vector<T> sorted = SortEdges(invPerms);
        T n                   = *sorted.begin();
        // visit(n)
        visit(n);
    }

    return L;
}

TIntermFunctionDefinition *CreateStructEqualityFunction(
    TSymbolTable &symbolTable,
    const TStructure &aStructType,
    const std::unordered_map<const TStructure *, const TFunction *> &equalityFunctions)
{
    auto &funcEquality =
        *new TFunction(&symbolTable, ImmutableString("equal"), SymbolType::AngleInternal,
                       new TType(TBasicType::EbtBool), true);

    auto &aStruct = CreateInstanceVariable(symbolTable, aStructType, Name("a"));
    auto &bStruct = CreateInstanceVariable(symbolTable, aStructType, Name("b"));
    funcEquality.addParameter(&aStruct);
    funcEquality.addParameter(&bStruct);

    auto &bodyEquality = *new TIntermBlock();
    std::vector<TIntermTyped *> andNodes;

    const TFieldList &aFields = aStructType.fields();
    const size_t size         = aFields.size();

    auto testEquality = [&](TIntermTyped &a, TIntermTyped &b) -> TIntermTyped * {
        ASSERT(a.getType() == b.getType());
        const TType &type = a.getType();
        if (const TStructure *structure = type.getStruct(); structure != nullptr)
        {
            auto func = equalityFunctions.find(structure);
            if (func != equalityFunctions.end())
            {
                return TIntermAggregate::CreateFunctionCall(*func->second,
                                                            new TIntermSequence{&a, &b});
            }
            UNREACHABLE();
        }
        return new TIntermBinary(TOperator::EOpEqual, &a, &b);
    };

    for (size_t idx = 0; idx < size; ++idx)
    {
        const TField &aField    = *aFields[idx];
        const TType &aFieldType = *aField.type();
        const Name aFieldName(aField);

        if (aFieldType.isArray())
        {
            ASSERT(!aFieldType.isArrayOfArrays());  // TODO
            int dim = aFieldType.getOutermostArraySize();
            for (int d = 0; d < dim; ++d)
            {
                auto &aAccess = AccessIndex(AccessField(aStruct, aFieldName), d);
                auto &bAccess = AccessIndex(AccessField(bStruct, aFieldName), d);
                auto *eqNode  = testEquality(bAccess, aAccess);
                andNodes.push_back(eqNode);
            }
        }
        else
        {
            auto &aAccess = AccessField(aStruct, aFieldName);
            auto &bAccess = AccessField(bStruct, aFieldName);
            auto *eqNode  = testEquality(bAccess, aAccess);
            andNodes.push_back(eqNode);
        }
    }

    ASSERT(andNodes.size() > 0);  // Empty structs are not allowed in GLSL
    TIntermTyped *outNode = andNodes.back();
    andNodes.pop_back();
    for (TIntermTyped *andNode : andNodes)
    {
        outNode = new TIntermBinary(TOperator::EOpLogicalAnd, andNode, outNode);
    }
    bodyEquality.appendStatement(new TIntermBranch(TOperator::EOpReturn, outNode));
    auto *funcProtoEquality = new TIntermFunctionPrototype(&funcEquality);
    return new TIntermFunctionDefinition(funcProtoEquality, &bodyEquality);
}

struct DeclaredStructure
{
    TIntermDeclaration *declNode;
    const TStructure *structure;
};

bool GetAsDeclaredStructure(SymbolEnv &symbolEnv, TIntermNode &node, DeclaredStructure &out)
{
    if (TIntermDeclaration *declNode = node.getAsDeclarationNode())
    {
        ASSERT(declNode->getChildCount() == 1);
        TIntermNode &childNode = *declNode->getChildNode(0);

        if (TIntermSymbol *symbolNode = childNode.getAsSymbolNode())
        {
            const TVariable &var = symbolNode->variable();
            const TType &type    = var.getType();
            if (const TStructure *structure = symbolEnv.remap(type.getStruct()))
            {
                if (type.isStructSpecifier())
                {
                    out.declNode  = declNode;
                    out.structure = structure;
                    return true;
                }
            }
        }
    }
    return false;
}

class FindStructEqualityUse : public TIntermTraverser
{
  public:
    SymbolEnv &mSymbolEnv;
    std::unordered_set<const TStructure *> mUsedStructs;

    FindStructEqualityUse(SymbolEnv &symbolEnv)
        : TIntermTraverser(false, false, true), mSymbolEnv(symbolEnv)
    {}

    bool visitBinary(Visit, TIntermBinary *binary) override
    {
        const TOperator op = binary->getOp();

        switch (op)
        {
            case TOperator::EOpEqual:
            case TOperator::EOpNotEqual:
            {
                const TType &leftType  = binary->getLeft()->getType();
                const TType &rightType = binary->getRight()->getType();
                ASSERT(leftType.getStruct() == rightType.getStruct());
                if (const TStructure *structure = mSymbolEnv.remap(leftType.getStruct()))
                {
                    useStruct(*structure);
                }
            }
            break;

            default:
                break;
        }

        return true;
    }

  private:
    void useStruct(const TStructure &structure)
    {
        if (mUsedStructs.insert(&structure).second)
        {
            for (const TField *field : structure.fields())
            {
                if (const TStructure *subStruct = mSymbolEnv.remap(field->type()->getStruct()))
                {
                    useStruct(*subStruct);
                }
            }
        }
    }
};

}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

bool sh::ToposortStructs(TCompiler &compiler,
                         SymbolEnv &symbolEnv,
                         TIntermBlock &root,
                         ProgramPreludeConfig &ppc)
{
    FindStructEqualityUse finder(symbolEnv);
    root.traverse(&finder);
    auto &usedStructs = finder.mUsedStructs;

    std::vector<DeclaredStructure> declaredStructs;
    std::vector<TIntermNode *> nonStructStmtNodes;

    {
        DeclaredStructure declaredStruct;
        const size_t stmtCount = root.getChildCount();
        for (size_t i = 0; i < stmtCount; ++i)
        {
            TIntermNode &stmtNode = *root.getChildNode(i);
            if (GetAsDeclaredStructure(symbolEnv, stmtNode, declaredStruct))
            {
                declaredStructs.push_back(declaredStruct);
            }
            else
            {
                nonStructStmtNodes.push_back(&stmtNode);
            }
        }
    }

    {
        std::vector<const TStructure *> structs;
        std::unordered_map<const TStructure *, DeclaredStructure> rawToDeclared;

        for (const DeclaredStructure &d : declaredStructs)
        {
            structs.push_back(d.structure);
            ASSERT(rawToDeclared.find(d.structure) == rawToDeclared.end());
            rawToDeclared[d.structure] = d;
        }

        // Note: Graph may contain more than only explicitly declared structures.
        Graph<const TStructure *> g                   = BuildGraph(symbolEnv, structs);
        std::vector<const TStructure *> sortedStructs = Toposort(g);
        ASSERT(declaredStructs.size() <= sortedStructs.size());

        declaredStructs.clear();
        for (const TStructure *s : sortedStructs)
        {
            auto it = rawToDeclared.find(s);
            if (it != rawToDeclared.end())
            {
                auto &d = it->second;
                ASSERT(d.declNode);
                declaredStructs.push_back(d);
            }
        }
    }

    {
        TIntermSequence newStmtNodes;
        std::unordered_map<const TStructure *, const TFunction *> equalityFunctions;
        for (auto &[declNode, structure] : declaredStructs)
        {
            newStmtNodes.push_back(declNode);
            if (usedStructs.find(structure) != usedStructs.end())
            {
                TIntermFunctionDefinition *eq = CreateStructEqualityFunction(
                    compiler.getSymbolTable(), *structure, equalityFunctions);
                newStmtNodes.push_back(eq);
                equalityFunctions[structure] = eq->getFunction();
            }
        }

        for (TIntermNode *stmtNode : nonStructStmtNodes)
        {
            ASSERT(stmtNode);
            newStmtNodes.push_back(stmtNode);
        }

        *root.getSequence() = newStmtNodes;
    }

    return compiler.validateAST(&root);
}
