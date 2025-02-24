//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_SEPARATEDECLARATIONS_H_
#define COMPILER_TRANSLATOR_TREEOPS_SEPARATEDECLARATIONS_H_

#include "common/angleutils.h"

namespace sh
{
class TCompiler;
class TIntermBlock;

// Transforms declarations so that in the end each declaration contains only one declarator.
// This is useful as an intermediate step when initialization needs to be separated from
// declaration, or when things need to be unfolded out of the initializer.
// Examples:
// Input:
//     int a[1] = int[1](1), b[1] = int[1](2);
// Output:
//     int a[1] = int[1](1);
//     int b[1] = int[1](2);
// Input:
//    struct S { vec3 d; } a, b;
// Output:
//    struct S { vec3 d; } a;
//    S b;
// Input:
//    struct { vec3 d; } a;
// Output (note: no change):
//    struct { vec3 d; } a;
// Input:
//    struct { vec3 d; } a, b;
// Output:
//    struct s1234 { vec3 d; } a;
//    s1234 b;
// Input:
//   struct Foo { int a; } foo();
// Output:
//   struct Foo { int a; };
//   Foo foo();
// Input with separateCompoundStructDeclarations=true:
//    struct S { vec3 d; } a;
// Output:
//    struct S { vec3 d; };
//    S a;
// Input with separateCompoundStructDeclarations=true:
//    struct S { vec3 d; } a, b;
// Output:
//    struct S { vec3 d; };
//    S a;
//    S b;
// Input with separateCompoundStructDeclarations=true:
//    struct { vec3 d; } a, b;
// Output:
//    struct s1234 { vec3 d; };
//    s1234 a;
//    s1234 b;
// Input with separateCompoundStructDeclarations=true:
//    struct { vec3 d; } a;
// Output (note: now, changes):
//    struct s1234 { vec3 d; };
//    s1234 a;

[[nodiscard]] bool SeparateDeclarations(TCompiler &compiler,
                                        TIntermBlock &root,
                                        bool separateCompoundStructDeclarations);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_SEPARATEDECLARATIONS_H_
