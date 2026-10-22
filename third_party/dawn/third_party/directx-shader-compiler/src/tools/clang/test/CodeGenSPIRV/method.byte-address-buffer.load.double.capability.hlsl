// RUN: %dxc -T cs_6_0 -E main -O0 %s -spirv | FileCheck %s

// CHECK-NOT: OpCapability Int64
// CHECK-DAG: OpCapability Float64
// CHECK-NOT: OpCapability Int64

RWByteAddressBuffer buffer;

[numthreads(1, 1, 1)]
void main() {
  double tmp;

// CHECK: [[addr1:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buffer %uint_0 [[addr1]]
// CHECK: [[word0:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK: [[addr2:%[0-9]+]] = OpIAdd %uint [[addr1]] %uint_1
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buffer %uint_0 [[addr2]]
// CHECK: [[word1:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK: [[addr3:%[0-9]+]] = OpIAdd %uint [[addr2]] %uint_1
// CHECK: [[merge:%[0-9]+]] = OpCompositeConstruct %v2uint [[word0]] [[word1]]
// CHECK: [[value:%[0-9]+]] = OpBitcast %double [[merge]]
// CHECK:                     OpStore %tmp [[value]]
  tmp = buffer.Load<double>(0);

// CHECK: [[value:%[0-9]+]] = OpLoad %double %tmp
// CHECK: [[merge:%[0-9]+]] = OpBitcast %v2uint [[value]]
// CHECK: [[word0:%[0-9]+]] = OpCompositeExtract %uint [[merge]] 0
// CHECK: [[word1:%[0-9]+]] = OpCompositeExtract %uint [[merge]] 1

// CHECK: [[addr1:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buffer %uint_0 [[addr1]]
// CHECK:                     OpStore [[ptr]] [[word0]]
// CHECK: [[addr2:%[0-9]+]] = OpIAdd %uint [[addr1]] %uint_1
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buffer %uint_0 [[addr2]]
// CHECK:                     OpStore [[ptr]] [[word1]]
// CHECK: [[addr3:%[0-9]+]] = OpIAdd %uint [[addr2]] %uint_1
  buffer.Store<double>(0, tmp);
}

