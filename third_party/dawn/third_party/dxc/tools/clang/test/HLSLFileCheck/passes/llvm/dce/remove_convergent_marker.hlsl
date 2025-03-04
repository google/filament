// RUN: %dxc /T ps_6_0  %s | FileCheck %s

// Tests that a convergent marker added by a derivative call
// is removed in time to let the rest of the dead code be removed


// Since everything is ignored, there should be no conditionals
// and just a single return void

// CHECK: void @main
// CHECK-NOT: br
// CHECK: storeOutput
// CHECK: ret void

float main(float d : DEPTH0) : SV_Target {
    if (d > 0)
      d = max(d, 3.0);
    ddx(d+1);
    return 0.0;
}
