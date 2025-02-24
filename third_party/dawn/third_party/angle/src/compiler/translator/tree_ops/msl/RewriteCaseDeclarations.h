//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_MSL_REWRITECASEDECLARATIONS_H_
#define COMPILER_TRANSLATOR_TREEOPS_MSL_REWRITECASEDECLARATIONS_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"

namespace sh
{

// EXAMPLE
//    switch (expr)
//    {
//      case 0:
//        int x = 0;
//        break;
//      case 1:
//        int y = 0;
//        {
//          int z = 0;
//        }
//        break;
//    }
// Becomes
//    {
//      int x;
//      int y;
//      switch (expr)
//      {
//        case 0:
//          x = 0;
//          break;
//        case 1:
//          y = 0;
//          {
//            int z = 0;
//          }
//          break;
//      }
//    }
[[nodiscard]] bool RewriteCaseDeclarations(TCompiler &compiler, TIntermBlock &root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_MSL_REWRITECASEDECLARATIONS_H_
