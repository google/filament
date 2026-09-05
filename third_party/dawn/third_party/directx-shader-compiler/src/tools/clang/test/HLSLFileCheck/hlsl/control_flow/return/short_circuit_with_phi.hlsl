// RUN: %dxc -E main -fcgl -opt-enable structurize-returns -T ps_6_0  -HV 2021 %s | FileCheck %s
// RUN: %dxc -E main -opt-enable structurize-returns -T ps_6_0  -HV 2021 %s | FileCheck %s -check-prefix=FULL

// FULL: @main

float main(uint x : X, uint y : Y, uint z : Z) : SV_Target {
  // CHECK: %[[bReturned:.*]] = alloca i1
  // CHECK: store i1 false, i1* %[[bReturned]]

  // CHECK-NOT: ret float
  // CHECK: [[label:.+]] ; preds =
  if (x) {

    // CHECK: [[label2:.+]] ; preds =
    if (y) {
      // CHECK: store float 1.000000e+00
      // CHECK: store i1 true, i1* %[[bReturned]]
      // CHECK: br
      return 1;

      // CHECK: [[label_bRet_cmp_false:.+]]: ; preds =
      // CHECK: %[[RET:.*]] = load i1, i1* %[[bReturned]]
      // CHECK: %[[NRET:.*]] = icmp ne i1 %[[RET]], false
      // CHECK: br i1 %[[NRET]],
    }

    // CHECK: [[if_end:.+]] ; preds =
    // CHECK: load i32
    // CHECK: %[[x_to_bool:.+]] = icmp
    // CHECK: br i1 %[[x_to_bool]],

    // CHECK: [[land_rhs:.+]] ; preds =
    // CHECK: load i32
    // CHECK: icmp
    // CHECK: br
    bool cond = x && y;

    // CHECK: [[land_end:.+]] ; preds =
    // CHECK-NOT: phi.+ %[[label_bRet_cmp_false]]
    // CHECK: phi

    // CHECK: icmp
    // CHECK: br i1
    if (cond) {
      // CHECK: [[then2:.+]] ; preds =
      // store float 2.000000e+00
      // CHECK: store i1 true, i1* %[[bReturned]]
      return 2;

      // CHECK: [[label_bRet_cmp_false_2:.+]]: ; preds =
      // CHECK: %[[RET2:.*]] = load i1, i1* %[[bReturned]]
      // CHECK: %[[NRET2:.*]] = icmp ne i1 %[[RET2]], false
      // CHECK: br i1 %[[NRET2]],
    }
  }

  return 0; 
}
