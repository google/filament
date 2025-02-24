// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

// TODO: FlagIsPublic is shown as FlagIsProtected|FlagIsPrivate.

// CHECK:             [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK:         [[fooName:%[0-9]+]] = OpString "foo"
// CHECK:        [[emptyStr:%[0-9]+]] = OpString ""
// CHECK:        [[mainName:%[0-9]+]] = OpString "main"

// CHECK:    [[int:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Signed
// CHECK:  [[float:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Float

// CHECK: [[fooFnType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeFunction FlagIsProtected|FlagIsPrivate %void [[int]] [[float]]
// CHECK:          [[source:%[0-9]+]] = OpExtInst %void [[set]] DebugSource
// CHECK: [[compilationUnit:%[0-9]+]] = OpExtInst %void [[set]] DebugCompilationUnit

// Check DebugFunction instructions
//
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugFunction [[fooName]] [[fooFnType]] [[source]] 25 1 [[compilationUnit]] [[emptyStr]] FlagIsProtected|FlagIsPrivate 26 %foo

// CHECK: [[float4:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeVector [[float]] 4
// CHECK: [[mainFnType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeFunction FlagIsProtected|FlagIsPrivate [[float4]] [[float4]]
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugFunction [[mainName]] [[mainFnType]] [[source]] 30 1 [[compilationUnit]] [[emptyStr]] FlagIsProtected|FlagIsPrivate 31 %src_main

void foo(int x, float y)
{
  x = x + y;
}

float4 main(float4 color : COLOR) : SV_TARGET
{
  bool condition = false;
  foo(1, color.x);
  return color;
}

