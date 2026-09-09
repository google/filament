// RUN: %dxc -E main -fcgl -opt-enable structurize-returns -T ps_6_0 %s | FileCheck %s

int i;

float main(float4 a:A) : SV_Target {
// Init bReturned.
// CHECK:%[[bReturned:.*]] = alloca i1
// CHECK:store i1 false, i1* %[[bReturned]]

// CHECK:switch
  switch (i) {
  default:
// CHECK: [[default:.*]] ; preds =
// set bReturned to true.
// CHECK:store i1 true, i1* %[[bReturned]]
    return sin(a.y);
  case 1:
// CHECK: [[case1:.*]] ; preds =
// set bReturned to true.
// CHECK:store i1 true, i1* %[[bReturned]]
    return cos(a.x);
  }
// CHECK: [[exit:.*]]  ; preds =
// CHECK-NOT:preds
// CHECK:ret float
}