// RUN: %dxc -E main -fcgl -opt-enable structurize-returns -T ps_6_0 %s | FileCheck %s

int i;
// CHECK:define float @main
float main(float4 a:A) : SV_Target {
// Init bReturned.
// CHECK:%[[bReturned:.*]] = alloca i1
// CHECK-NEXT:store i1 false, i1* %[[bReturned]]
  float r = a.w;

// CHECK: [[if_then:.*]] ; preds =
  if (a.z > 0) {
// CHECK: [[for_cond:.*]] ; preds =
    for (int j=0;j<i;j++) {
// CHECK: [[for_body:.*]] ; preds =
// set bReturned to true.
// CHECK:store i1 true, i1* %[[bReturned]]
// dxBreak
// CHECK:br i1 true
       return log(i);

// CHECK: [[for_inc:.*]] ; preds =
    }
// guard rest of scope with !bReturned
// CHECK: [[bRet_cmp_false:.*]] ; preds =
// CHECK:%[[RET:.*]] = load i1, i1* %[[bReturned]]
// CHECK:%[[NRET:.*]] = icmp ne i1 %[[RET]], false
// CHECK:br i1 %[[NRET]],

// CHECK: [[for_end:.*]]  ; preds =
    r += sin(a.y);
// set bReturned to true.
// CHECK:store i1 true, i1* %[[bReturned]]
    return sin(a.x * a.z + r);
  } else {
// CHECK: [[else:.*]]  ; preds =
// set bReturned to true.
// CHECK:store i1 true, i1* %[[bReturned]]
    return cos(r + a.z);
  }

// dead code which not has code generated.
  return a.x + a.y;

// CHECK: [[exit:.*]]  ; preds =
// CHECK-NOT:preds
// CHECK:ret float
}