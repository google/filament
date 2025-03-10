// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

float4 main(float4 input : A) : SV_Target {
  float2x2 floatMat;
  int2x2   intMat;
  bool2x2  boolMat;

// CHECK:      [[floatMat:%[0-9]+]] = OpLoad %mat2v2float %floatMat
// CHECK-NEXT:     [[row0:%[0-9]+]] = OpCompositeExtract %v2float [[floatMat]] 0
// CHECK-NEXT:     [[row1:%[0-9]+]] = OpCompositeExtract %v2float [[floatMat]] 1
// CHECK-NEXT:      [[vec:%[0-9]+]] = OpVectorShuffle %v4float [[row0]] [[row1]] 0 1 2 3
// CHECK-NEXT:                     OpStore %c [[vec]]
  float4 c = floatMat;

// CHECK:        [[intMat:%[0-9]+]] = OpLoad %_arr_v2int_uint_2 %intMat
// CHECK-NEXT:     [[row0_0:%[0-9]+]] = OpCompositeExtract %v2int [[intMat]] 0
// CHECK-NEXT:     [[row1_0:%[0-9]+]] = OpCompositeExtract %v2int [[intMat]] 1
// CHECK-NEXT:   [[vecInt:%[0-9]+]] = OpVectorShuffle %v4int [[row0_0]] [[row1_0]] 0 1 2 3
// CHECK-NEXT: [[vecFloat:%[0-9]+]] = OpConvertSToF %v4float [[vecInt]]
// CHECK-NEXT:                     OpStore %d [[vecFloat]]
  float4 d = intMat;

// CHECK:       [[boolMat:%[0-9]+]] = OpLoad %_arr_v2bool_uint_2 %boolMat
// CHECK-NEXT:     [[row0_1:%[0-9]+]] = OpCompositeExtract %v2bool [[boolMat]] 0
// CHECK-NEXT:     [[row1_1:%[0-9]+]] = OpCompositeExtract %v2bool [[boolMat]] 1
// CHECK-NEXT:      [[vec_0:%[0-9]+]] = OpVectorShuffle %v4bool [[row0_1]] [[row1_1]] 0 1 2 3
// CHECK-NEXT: [[vecFloat_0:%[0-9]+]] = OpSelect %v4float [[vec_0]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:                     OpStore %e [[vecFloat_0]]
  float4 e = boolMat;

  return 0.xxxx;
}

