// RUN: not %dxc -T cs_6_0 -E main -Zi -fcgl  %s -spirv  2>&1 | FileCheck %s

#include "DoesntExist.hlsl"

void main() {}


// CHECK: 3:10: fatal error: 'DoesntExist.hlsl' file not found
