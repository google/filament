// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"

// CHECK: rich.debug.function.parent.hlsl
// CHECK: spirv.debug.opline.include-file-1.hlsli
// CHECK: spirv.debug.opline.include-file-2.hlsli
// CHECK: spirv.debug.opline.include-file-3.hlsli
// CHECK: [[f3:%[0-9]+]] = OpString "function3"
// CHECK: [[f2:%[0-9]+]] = OpString "function2"
// CHECK: [[f1:%[0-9]+]] = OpString "function1"


#include "spirv.debug.opline.include-file-1.hlsli"

int callFunction1() {
  return function1();
}

#include "spirv.debug.opline.include-file-2.hlsli"

int callFunction2() {
  return function2();
}

#include "spirv.debug.opline.include-file-3.hlsli"

int callFunction3() {
  CALL_FUNCTION_3;
}


// CHECK: [[main:%[0-9]+]] = OpExtInst %void [[set]] DebugSource
// CHECK: [[c3:%[0-9]+]] = OpExtInst %void [[set]] DebugCompilationUnit 1 4 [[main]] HLSL
// CHECK-NOT: DebugCompilationUnit
// CHECK: [[s3:%[0-9]+]] = OpExtInst %void [[set]] DebugSource
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugFunction [[f3]] {{%[0-9]+}} [[s3]] 3 1 [[c3]]

// CHECK: [[s2:%[0-9]+]] = OpExtInst %void [[set]] DebugSource
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugFunction [[f2]] {{%[0-9]+}} [[s2]] 2 1 [[c3]]

// CHECK: [[s1:%[0-9]+]] = OpExtInst %void [[set]] DebugSource
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugFunction [[f1]] {{%[0-9]+}} [[s1]] 1 1 [[c3]]

void main() {
  callFunction1();
  callFunction2();
  callFunction3();
}
