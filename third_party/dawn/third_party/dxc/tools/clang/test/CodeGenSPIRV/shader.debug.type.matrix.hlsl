// RUN: %dxc -T ps_6_0 -E main -fspv-debug=vulkan -fcgl  %s -spirv | FileCheck %s
// RUN: %dxc -T ps_6_2 -E main -fspv-debug=vulkan -fcgl  %s -spirv -enable-16bit-types | FileCheck %s --check-prefix=CHECK-HALF

// CHECK: [[float:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugTypeBasic {{%[0-9]+}} %uint_32 %uint_3 %uint_0
// CHECK: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugTypeVector {{%[0-9]+}} %uint_4
// CHECK: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugTypeMatrix {{%[0-9]+}} %uint_3 %uint_1
// CHECK: [[float:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugTypeBasic {{%[0-9]+}} %uint_64 %uint_3 %uint_0
// CHECK: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugTypeVector {{%[0-9]+}} %uint_4
// CHECK: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugTypeMatrix {{%[0-9]+}} %uint_3 %uint_1
// CHECK-HALF: [[float:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugTypeBasic {{%[0-9]+}} %uint_16 %uint_3 %uint_0
// CHECK-HALF: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugTypeVector {{%[0-9]+}} %uint_4
// CHECK-HALF: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugTypeMatrix {{%[0-9]+}} %uint_3 %uint_1
// CHECK-HALF: [[float:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugTypeBasic {{%[0-9]+}} %uint_64 %uint_3 %uint_0
// CHECK-HALF: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugTypeVector {{%[0-9]+}} %uint_4
// CHECK-HALF: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugTypeMatrix {{%[0-9]+}} %uint_3 %uint_1
// CHECK-HALF: [[float:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugTypeBasic {{%[0-9]+}} %uint_32 %uint_3 %uint_0
// CHECK-HALF: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugTypeVector {{%[0-9]+}} %uint_4
// CHECK-HALF: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugTypeMatrix {{%[0-9]+}} %uint_3 %uint_1

void main() {
   float3x4 mat_float;
   double3x4 mat_double;
   half3x4 mat_half;
}
