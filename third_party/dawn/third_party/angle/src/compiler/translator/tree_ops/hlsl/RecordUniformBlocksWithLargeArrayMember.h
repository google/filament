//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RecordUniformBlocksWithLargeArrayMember.h:
// Collect all uniform blocks which have one or more large array members,
// and the array sizes are greater than or equal to 50. If some of them
// satify some conditions, we will translate them to StructuredBuffers
// on Direct3D backend.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_HLSL_RECORDUNIFORMBLOCKSWITHLARGEARRAYMEMBER_H_
#define COMPILER_TRANSLATOR_TREEOPS_HLSL_RECORDUNIFORMBLOCKSWITHLARGEARRAYMEMBER_H_

#include "compiler/translator/IntermNode.h"

namespace sh
{
class TIntermNode;

[[nodiscard]] bool RecordUniformBlocksWithLargeArrayMember(
    TIntermNode *root,
    std::map<int, const TInterfaceBlock *> &uniformBlockOptimizedMap,
    std::set<std::string> &slowCompilingUniformBlockSet);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_HLSL_RECORDUNIFORMBLOCKSWITHLARGEARRAYMEMBER_H_
