// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Signatures:
// double  asdouble(uint lowbits, uint highbits)
// double2 asdouble(uint2 lowbits, uint2 highbits)

void main() {

// CHECK:      [[arg:%[0-9]+]] = OpCompositeConstruct %v2uint %uint_1 %uint_2
// CHECK-NEXT:     {{%[0-9]+}} = OpBitcast %double [[arg]]
  double a = asdouble(1u, 2u);

  uint low, high;
// CHECK:        [[low:%[0-9]+]] = OpLoad %uint %low
// CHECK-NEXT:  [[high:%[0-9]+]] = OpLoad %uint %high
// CHECK-NEXT:  [[arg2:%[0-9]+]] = OpCompositeConstruct %v2uint [[low]] [[high]]
// CHECK-NEXT:       {{%[0-9]+}} = OpBitcast %double [[arg2]]
  double b = asdouble(low, high);

// CHECK:         [[low2:%[0-9]+]] = OpLoad %v2uint %low2
// CHECK-NEXT:   [[high2:%[0-9]+]] = OpLoad %v2uint %high2
// CHECK-NEXT:    [[arg3:%[0-9]+]] = OpVectorShuffle %v4uint [[low2]] [[high2]] 0 2 1 3
// CHECK-NEXT:         {{%[0-9]+}} = OpBitcast %v2double [[arg3]]
  uint2 low2, high2;
  double2 c = asdouble(low2, high2);
}
