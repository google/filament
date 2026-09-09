// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  int   sint_1;
  int4  sint_4;
  uint  uint_1;
  uint2 uint_2;
  uint4 uint_4;

// CHECK: [[sint_1:%[0-9]+]] = OpLoad %int %sint_1
// CHECK:    [[msb:%[0-9]+]] = OpExtInst %uint [[glsl]] FindSMsb [[sint_1]]
// CHECK:    [[res:%[0-9]+]] = OpBitcast %int [[msb]]
// CHECK:                   OpStore %fbh [[res]]
  int fbh = firstbithigh(sint_1);

// CHECK: [[sint_4:%[0-9]+]] = OpLoad %v4int %sint_4
// CHECK:    [[msb:%[0-9]+]] = OpExtInst %v4uint [[glsl]] FindSMsb [[sint_4]]
// CHECK:    [[res:%[0-9]+]] = OpBitcast %v4int [[msb]]
// CHECK:                   OpStore %fbh4 [[res]]
  int4 fbh4 = firstbithigh(sint_4);

// CHECK: [[uint_1:%[0-9]+]] = OpLoad %uint %uint_1
// CHECK:    [[msb:%[0-9]+]] = OpExtInst %uint [[glsl]] FindUMsb [[uint_1]]
// CHECK:                   OpStore %ufbh [[msb]]
  uint ufbh = firstbithigh(uint_1);

// CHECK: [[uint_2:%[0-9]+]] = OpLoad %v2uint %uint_2
// CHECK:    [[msb:%[0-9]+]] = OpExtInst %v2uint [[glsl]] FindUMsb [[uint_2]]
// CHECK:                   OpStore %ufbh2 [[msb]]
  uint2 ufbh2 = firstbithigh(uint_2);

// CHECK: [[uint_4:%[0-9]+]] = OpLoad %v4uint %uint_4
// CHECK:    [[msb:%[0-9]+]] = OpExtInst %v4uint [[glsl]] FindUMsb [[uint_4]]
// CHECK:                   OpStore %ufbh4 [[msb]]
  uint4 ufbh4 = firstbithigh(uint_4);
}
