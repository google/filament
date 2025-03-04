// Test switch with multiple same successors
// RUN: %dxc -T ps_6_6 %s | FileCheck %s

// This test used to fail with validation errors:
//
// error: validation errors
// error: Module bitcode is invalid.
// error: PHINode should have one entry for each predecessor of its parent basic block!
//   %22 = phi i32 [ 1, %20 ], [ 1, %20 ], [ 1, %20 ], [ 1, %20 ], [ %11, %13 ]
// PHINode should have one entry for each predecessor of its parent basic block!
//   %28 = phi i32 [ 1, %26 ], [ 1, %26 ], [ 1, %26 ], [ 1, %26 ], [ %22, %24 ]
// PHINode should have one entry for each predecessor of its parent basic block!
//   %34 = phi i32 [ 1, %32 ], [ 1, %32 ], [ 1, %32 ], [ 1, %32 ], [ %28, %30 ]
// PHINode should have one entry for each predecessor of its parent basic block!
//   %47 = phi i32 [ 1, %45 ], [ 1, %45 ], [ 1, %45 ], [ 1, %45 ], [ %41, %43 ]
//
// This was fixed in dxil-remove-dead-blocks. See switch-with-multiple-same-successor.ll
// for the pass-specific checks. Here, we just want to make sure dxc compiles this without error.

// CHECK: @main

ByteAddressBuffer g_buff : register(t0);

struct retval {
  float4 value : SV_Target0;
};

retval main() {
  float4 g = asfloat(g_buff.Load4(0u));
  bool do_discard = false;

  for (int i = 0; i < 10; ++i) {
    if (g.x != 0.0f)
      continue;

    // Switch with the same successor in all cases
    switch(i) {
      case 1: {
        g.x = g.x;
        break;
      }
      case 2: {
        g.x = g.x;
        break;
      }
      case 3: {
        g.x = g.x;
        break;
      }
      // Skip 'case 4' to avoid case range combining
      case 5: {
        g.x = g.x;
        break;
      }
    }
    if (i == 6) {
      break;
    }
    g.x = atan2(1.0f, g.x);
    do_discard = true;
  }

  if (do_discard) {
    discard;
  }
  
  return (retval)0;
}
