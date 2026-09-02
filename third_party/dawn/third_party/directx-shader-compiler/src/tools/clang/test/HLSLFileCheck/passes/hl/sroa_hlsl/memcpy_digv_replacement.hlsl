// RUN: %dxc -T vs_6_0 -Zi %s | FileCheck %s

// Test replacement of static global in a case where SROA may try to
// process a static global replaced by memcpy lowering.
// This processing includes creating flattened GVs for it
// and then updating the debug info for them with the original DIGV,
// which no longer matches because of the replacement, so null is found
// and unnecessary flattened variables were created.
// Trivially dead GVs are now removed in SROA

// Ensure the replacement var is flattened
// CHECK: ReplacementVar.0 = internal unnamed_addr constant [2 x float]
// CHECK: ReplacementVar.1 = internal unnamed_addr constant [2 x float]
// CHECK: ReplacementVar.2 = internal unnamed_addr constant [2 x float]
// CHECK: ReplacementVar.3 = internal unnamed_addr constant [2 x float]
// Ensure there are no flattened variables created for the replaced GV
// CHECK-NOT: UnusedVar.[0-3]

// CHECK: @main

// Ensure the replacement var gets its debug info
// CHECK: DIGlobalVariable(name: "ReplacementVar",
// CHECK: DIGlobalVariable(name: "ReplacementVar.0",
// CHECK: DIGlobalVariable(name: "ReplacementVar.1",
// CHECK: DIGlobalVariable(name: "ReplacementVar.2",
// CHECK: DIGlobalVariable(name: "ReplacementVar.3",

// Ensure there are no DI for flattened variables created for the replaced GV
// CHECK-NOT: DIGlobalVariable(name: "UnusedVar.[0-3]",

// All const does is force ReplacementVar to be processed first, which
// replaces all uses of OrigVar, but leaves OrigVar in the worklist
// which causes the aforementioned problems when it is reached
// Otherwise, OrigVar would be encountered first and it would be
// replaced at the same time as it was retrieved and removed from the worklist
static const float4 ReplacementVar[2] = { float4(1.0f, 2.0f, 3.0f, 4.0f),
                                          float4(9.0f, 8.0f, 7.0f, 6.0f) };
static float4 OrigVar[2];

float4 main(int ix : I) : SV_Position
{
  OrigVar = ReplacementVar;
  return OrigVar[ix];
}

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}
