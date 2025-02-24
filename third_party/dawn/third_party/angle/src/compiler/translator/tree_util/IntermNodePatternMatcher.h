//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IntermNodePatternMatcher is a helper class for matching node trees to given patterns.
// It can be used whenever the same checks for certain node structures are common to multiple AST
// traversers.
//

#ifndef COMPILER_TRANSLATOR_TREEUTIL_INTERMNODEPATTERNMATCHER_H_
#define COMPILER_TRANSLATOR_TREEUTIL_INTERMNODEPATTERNMATCHER_H_

namespace sh
{

class TIntermAggregate;
class TIntermBinary;
class TIntermDeclaration;
class TIntermNode;
class TIntermTernary;
class TIntermUnary;

class IntermNodePatternMatcher
{
  public:
    static bool IsDynamicIndexingOfNonSSBOVectorOrMatrix(TIntermBinary *node);
    static bool IsDynamicIndexingOfVectorOrMatrix(TIntermBinary *node);
    static bool IsDynamicIndexingOfSwizzledVector(TIntermBinary *node);

    enum PatternType : unsigned int
    {
        // Matches expressions that are unfolded to if statements by UnfoldShortCircuitToIf
        kUnfoldedShortCircuitExpression = 1u << 0u,

        // Matches expressions that return arrays with the exception of simple statements where a
        // constructor or function call result is assigned.
        kExpressionReturningArray = 1u << 1u,

        // Matches dynamic indexing of vectors or matrices in l-values.
        kDynamicIndexingOfVectorOrMatrixInLValue = 1u << 2u,

        // Matches declarations with more than one declared variables.
        kMultiDeclaration = 1u << 3u,

        // Matches declarations of arrays.
        kArrayDeclaration = 1u << 4u,

        // Matches declarations of structs where the struct type does not have a name.
        kNamelessStructDeclaration = 1u << 5u,

        // Matches array length() method.
        kArrayLengthMethod = 1u << 6u,
    };
    IntermNodePatternMatcher(const unsigned int mask);

    bool match(TIntermUnary *node) const;

    bool match(TIntermBinary *node, TIntermNode *parentNode) const;

    // Use this version for checking binary node matches in case you're using flag
    // kDynamicIndexingOfVectorOrMatrixInLValue.
    bool match(TIntermBinary *node, TIntermNode *parentNode, bool isLValueRequiredHere) const;

    bool match(TIntermAggregate *node, TIntermNode *parentNode) const;
    bool match(TIntermTernary *node) const;
    bool match(TIntermDeclaration *node) const;

  private:
    const unsigned int mMask;

    bool matchInternal(TIntermBinary *node, TIntermNode *parentNode) const;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEUTIL_INTERMNODEPATTERNMATCHER_H_
