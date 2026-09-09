// RUN: %dxc -E main -T ps_6_0 -spirv -fcgl -fspv-debug=vulkan  %s | FileCheck %s

// TODO: FlagIsPublic is shown as FlagIsProtected|FlagIsPrivate.

// CHECK:             [[set:%[0-9]+]] = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
// CHECK:         [[fooName:%[0-9]+]] = OpString "foo"
// CHECK:        [[emptyStr:%[0-9]+]] = OpString ""
// CHECK:     [[srcMainName:%[0-9]+]] = OpString "main"
// CHECK:        [[mainName:%[0-9]+]] = OpString "__dxc_setup"
// CHECK:          [[clOpts:%[0-9]+]] = OpString " -E main -T ps_6_0 -spirv -fcgl -fspv-debug=vulkan

// CHECK:    [[int:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 %uint_4 %uint_0
// CHECK:  [[float:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 %uint_3 %uint_0

// CHECK: [[fooFnType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeFunction %uint_3 %void [[int]] [[float]]
// CHECK:          [[source:%[0-9]+]] = OpExtInst %void [[set]] DebugSource
// CHECK: [[compilationUnit:%[0-9]+]] = OpExtInst %void [[set]] DebugCompilationUnit

// Check DebugFunction instructions
//
// CHECK: [[float4:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeVector [[float]] %uint_4
// CHECK: [[srcMainFnType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeFunction %uint_3 [[float4]] [[float4]]
// CHECK: [[srcMainDbgFn:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction [[srcMainName]] [[srcMainFnType]] [[source]] %uint_46 %uint_1 [[compilationUnit]] [[emptyStr]] %uint_3 %uint_47 
// CHECK: [[mainFnType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeFunction %uint_3 %void
// CHECK: [[mainDbgFn:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction [[mainName]] [[mainFnType]] [[source]] %uint_46 %uint_1 [[compilationUnit]] [[emptyStr]] %uint_3 %uint_47 
// CHECK: [[mainDbgEp:%[0-9]+]] = OpExtInst %void [[set]] DebugEntryPoint [[mainDbgFn]] [[compilationUnit]] {{%[0-9]+}} [[clOpts]]

// Check DebugFunctionDefinition is in main
//
// CHECK: %main = OpFunction %void None {{%[0-9]+}}
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugFunctionDefinition [[mainDbgFn]] %main
// CHECK: OpFunctionEnd

// Check DebugFunctionDefinition is in src.main
//
// CHECK: %src_main = OpFunction %v4float None {{%[0-9]+}}
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugScope [[srcMainDbgFn]]
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugFunctionDefinition [[srcMainDbgFn]] %src_main
// CHECK: OpFunctionEnd

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

