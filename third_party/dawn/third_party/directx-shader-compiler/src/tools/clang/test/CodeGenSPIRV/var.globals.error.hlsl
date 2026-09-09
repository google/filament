// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

[[vk::binding(10, 2)]] float4 gVec = 1.0;

float4 main() : A { return gVec; }

// CHECK: :3:38: warning: variable 'gVec' will be placed in $Globals so initializer ignored
// CHECK: :3:3: error: variable 'gVec' will be placed in $Globals so cannot have vk::binding attribute
