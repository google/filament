// RUN: %dxc -E main -fcgl -opt-enable structurize-returns -T ps_6_0 %s | FileCheck %s

float main(float4 a:A) : SV_Target {
// Init bReturned.
// CHECK:%[[bReturned:.*]] = alloca i1
// CHECK:store i1 false, i1* %[[bReturned]]

// Init retVal to undef.
// CHECK:store float undef

// CHECK: [[label:.*]] ; preds =
  if (a.w < 0) {
// CHECK: [[label2:.*]] ; preds =
   if (floor(a.x) > 1) {
// set bReturned to true.
// CHECK:store i1 true, i1* %[[bReturned]]
     return sin(a.y);
   } else {
// CHECK: [[else:.*]] ; preds =
// set bReturned to true.
// CHECK:store i1 true, i1* %[[bReturned]]
     float r = log(a.z);
     return r;
   }
  }
// guard rest of scope with !bReturned
// CHECK: [[label_bRet_cmp_false:.*]] ; preds =
// CHECK:%[[RET:.*]] = load i1, i1* %[[bReturned]]
// CHECK:%[[NRET:.*]] = icmp ne i1 %[[RET]], false
// CHECK:br i1 %[[NRET]],

// CHECK: [[label3:.*]]  ; preds =
  return cos(a.x+a.y);
// CHECK: [[exit:.*]]  ; preds =
// CHECK-NOT:preds
// CHECK:ret float
}