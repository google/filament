// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich-with-source -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[debugSet:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"

// CHECK: rich.debug.debugsource.multiple.hlsl
// CHECK: spirv.debug.opline.include-file-1.hlsli
// CHECK: spirv.debug.opline.include-file-2.hlsli
// CHECK: spirv.debug.opline.include-file-3.hlsli
// CHECK: [[main_code:%[0-9]+]] = OpString "// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich-with-source -fcgl  %s -spirv | FileCheck %s
// CHECK: [[file3_code:%[0-9]+]] = OpString "int b;
// CHECK: [[file2_code:%[0-9]+]] = OpString "static int a;
// CHECK: [[file1_code:%[0-9]+]] = OpString "int function1() {


// CHECK: {{%[0-9]+}} = OpExtInst %void [[debugSet]] DebugSource {{%[0-9]+}} [[main_code]]
// CHECK: {{%[0-9]+}} = OpExtInst %void [[debugSet]] DebugSource {{%[0-9]+}} [[file3_code]]
// CHECK: {{%[0-9]+}} = OpExtInst %void [[debugSet]] DebugSource {{%[0-9]+}} [[file2_code]]
// CHECK: {{%[0-9]+}} = OpExtInst %void [[debugSet]] DebugSource {{%[0-9]+}} [[file1_code]]

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

void main() {
  callFunction1();
  callFunction2();
  callFunction3();
}
