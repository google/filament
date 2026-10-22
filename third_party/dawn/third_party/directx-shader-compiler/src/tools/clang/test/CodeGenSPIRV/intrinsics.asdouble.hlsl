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
// CHECK-NEXT:  [[elem1:%[0-9]+]] = OpVectorShuffle %v2uint [[low2]] [[high2]] 0 2
// CHECK-NEXT:      [[double1:%[0-9]+]] = OpBitcast %double [[elem1]]
// CHECK-NEXT:  [[elem2:%[0-9]+]] = OpVectorShuffle %v2uint [[low2]] [[high2]] 1 3
// CHECK-NEXT:      [[double2:%[0-9]+]] = OpBitcast %double [[elem2]]
// CHECK-NEXT:   {{%[0-9]+}} = OpCompositeConstruct %v2double [[double1]] [[double2]]
  uint2 low2, high2;
  double2 c = asdouble(low2, high2);

// CHECK:         [[low3:%[0-9]+]] = OpLoad %v3uint %low3
// CHECK-NEXT:   [[high3:%[0-9]+]] = OpLoad %v3uint %high3
// CHECK-NEXT:  [[elem1:%[0-9]+]] = OpVectorShuffle %v2uint [[low3]] [[high3]] 0 3
// CHECK-NEXT:      [[double1:%[0-9]+]] = OpBitcast %double [[elem1]]
// CHECK-NEXT:  [[elem2:%[0-9]+]] = OpVectorShuffle %v2uint [[low3]] [[high3]] 1 4
// CHECK-NEXT:      [[double2:%[0-9]+]] = OpBitcast %double [[elem2]]
// CHECK-NEXT:  [[elem3:%[0-9]+]] = OpVectorShuffle %v2uint [[low3]] [[high3]] 2 5
// CHECK-NEXT:      [[double3:%[0-9]+]] = OpBitcast %double [[elem3]]
// CHECK-NEXT:   {{%[0-9]+}} = OpCompositeConstruct %v3double [[double1]] [[double2]] [[double3]]
  uint3 low3, high3;
  double3 d = asdouble(low3, high3);

// CHECK:         [[low4:%[0-9]+]] = OpLoad %v4uint %low4
// CHECK-NEXT:   [[high4:%[0-9]+]] = OpLoad %v4uint %high4
// CHECK-NEXT:  [[elem1:%[0-9]+]] = OpVectorShuffle %v2uint [[low4]] [[high4]] 0 4
// CHECK-NEXT:      [[double1:%[0-9]+]] = OpBitcast %double [[elem1]]
// CHECK-NEXT:  [[elem2:%[0-9]+]] = OpVectorShuffle %v2uint [[low4]] [[high4]] 1 5
// CHECK-NEXT:      [[double2:%[0-9]+]] = OpBitcast %double [[elem2]]
// CHECK-NEXT:  [[elem3:%[0-9]+]] = OpVectorShuffle %v2uint [[low4]] [[high4]] 2 6
// CHECK-NEXT:      [[double3:%[0-9]+]] = OpBitcast %double [[elem3]]
// CHECK-NEXT:  [[elem4:%[0-9]+]] = OpVectorShuffle %v2uint [[low4]] [[high4]] 3 7
// CHECK-NEXT:      [[double4:%[0-9]+]] = OpBitcast %double [[elem4]]
// CHECK-NEXT:   {{%[0-9]+}} = OpCompositeConstruct %v4double [[double1]] [[double2]] [[double3]] [[double4]]
  uint4 low4, high4;
  double4 e = asdouble(low4, high4);
}
