// RUN: %dxc -T ps_6_0 -E main -fspv-debug=vulkan -fcgl  %s -spirv | FileCheck %s

// CHECK:           [[main:%[0-9]+]] = OpString
// CHECK-SAME:      shader.debug.line.include.hlsl
// CHECK:           OpString
// CHECK:           [[file3:%[0-9]+]] = OpString
// CHECK-SAME:      spirv.debug.opline.include-file-3.hlsli
// CHECK:           OpString
// CHECK:           OpString
// CHECK:           [[file2:%[0-9]+]] = OpString
// CHECK-SAME:      spirv.debug.opline.include-file-2.hlsli
// CHECK:           OpString
// CHECK:           [[file1:%[0-9]+]] = OpString
// CHECK-SAME:      spirv.debug.opline.include-file-1.hlsli
// CHECK:      [[src0:%[0-9]+]] = OpExtInst %void %1 DebugSource [[main]]
// CHECK:      [[src3:%[0-9]+]] = OpExtInst %void %1 DebugSource [[file3]]
// CHECK:      [[src2:%[0-9]+]] = OpExtInst %void %1 DebugSource [[file2]]
// CHECK:      [[src1:%[0-9]+]] = OpExtInst %void %1 DebugSource [[file1]]

// DebugLine cannot preceed OpFunction
// CHECK:      %src_main = OpFunction %void None

#include "spirv.debug.opline.include-file-1.hlsli"

int callFunction1() {
  return function1();
}

#include "spirv.debug.opline.include-file-2.hlsli"

int callFunction2() {
  // This
  // is
  // an
  // intentional
  // multiple
  // lines.
  // It
  // might
  // be
  // a
  // single
  // line
  // in
  // OpSource.
  return function2();
}

#include "spirv.debug.opline.include-file-3.hlsli"

int callFunction3() {
  // This
  // is
  // an
  // intentional
  // multiple
  // lines.
  // It
  // might
  // be
  // a
  // single
  // line
  // in
  // OpSource.
  CALL_FUNCTION_3;
}

void main() {
// CHECK:      DebugLine [[src0]] %uint_72 %uint_72 %uint_3 %uint_17
// CHECK-NEXT: OpFunctionCall %int %callFunction1
  callFunction1();

  // This
  // is
  // an
  // intentional
  // multiple
  // lines.
  // It
  // might
  // be
  // a
  // single
  // line
  // in
  // OpSource.
// CHECK:      DebugLine [[src0]] %uint_90 %uint_90 %uint_3 %uint_17
// CHECK-NEXT: OpFunctionCall %int %callFunction2
  callFunction2();

// CHECK:      DebugLine [[src0]] %uint_94 %uint_94 %uint_3 %uint_17
// CHECK-NEXT: OpFunctionCall %int %callFunction3
  callFunction3();
}

// CHECK:      %function1 = OpFunction %int None
// CHECK:      DebugLine [[src1]] %uint_2 %uint_2 %uint_3 %uint_10
// CHECK-NEXT: OpReturnValue %int_1

// CHECK:      %function2 = OpFunction %int None
// CHECK:      DebugLine [[src2]] %uint_3 %uint_3 %uint_10 %uint_10
// CHECK-NEXT: OpLoad %int %a

// CHECK:      %function3 = OpFunction %int None
// CHECK:      DebugLine [[src3]] %uint_4 %uint_4 %uint_10 %uint_10
// CHECK:      OpLoad %int
