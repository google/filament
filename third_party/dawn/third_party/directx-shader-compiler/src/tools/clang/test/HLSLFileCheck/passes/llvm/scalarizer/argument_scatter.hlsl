// RUN: %dxc -T lib_6_3 %s | FileCheck %s

// Test for failure where the extractelement ops ended up after the loop phi that used them
// Resulted in a crash in LCSSA, but the order could have been out of whack regardless

// make sure the four extractions come before their usage in the loop
// CHECK: [[X:%[A-Za-z0-9\.]+]] = extractelement <4 x float> {{%?[A-Za-z0-9]+}}, i32 0
// CHECK: [[Y:%[A-Za-z0-9\.]+]] = extractelement <4 x float> {{%?[A-Za-z0-9]+}}, i32 1
// CHECK: [[Z:%[A-Za-z0-9\.]+]] = extractelement <4 x float> {{%?[A-Za-z0-9]+}}, i32 2
// CHECK: [[W:%[A-Za-z0-9\.]+]] = extractelement <4 x float> {{%?[A-Za-z0-9]+}}, i32 3

// CHECK: phi float {{.*}}[[X]],
// CHECK: phi float {{.*}}[[Y]],
// CHECK: phi float {{.*}}[[Z]],
// CHECK: phi float {{.*}}[[W]],

export
float4 ScatterLoop(float4 color, int ct) {
  // surprised this can't be unrolled
  for( int i = 0; i < ct; i++ )
    color += 1;
  return color;
}
