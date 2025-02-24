// RUN: %dxc -E main -fcgl -opt-enable structurize-returns -T ps_6_0 %s | FileCheck %s

int i;

float main(float4 a:A) : SV_Target {
// Init bReturned.
// CHECK:%[[bReturned:.*]] = alloca i1
// CHECK:store i1 false, i1* %[[bReturned]]
// Init retVal to 0.
// CHECK:store float 0.000000e+00

  float c = 0;

// CHECK: [[label:.*]] ; preds =
  if (i < 0) {
// CHECK: [[label2:.*]] ; preds =
    if (a.w > 2)
// return inside if.
// set bReturned to true.
// CHECK:store i1 true, i1* %[[bReturned]]
      return -1;
// guard rest of scope with !bReturned
// CHECK: [[label_bRet_cmp_false:.*]] ; preds =
// CHECK:%[[RET:.*]] = load i1, i1* %[[bReturned]]
// CHECK:%[[NRET:.*]] = icmp ne i1 %[[RET]], false
// CHECK:br i1 %[[NRET]],

// CHECK: [[label3:.*]]  ; preds =
    c += sin(a.z);
  }
  else {
// CHECK: [[else:.*]] ; preds =
    if (a.z > 3)
// return inside else
// set bIsReturn to true
// CHECK:store i1 true, i1* %[[bReturned]]
      return -5;
// CHECK: [[label_bRet_cmp_false2:.*]] ; preds =
// CHECK:%[[RET2:.*]] = load i1, i1* %[[bReturned]]
// CHECK:%[[NRET2:.*]] = icmp ne i1 %[[RET2]], false
// CHECK:br i1 %[[NRET2]],

// CHECK: [[label4:.*]] ; preds =
    c *= cos(a.w);
// guard after endif.
// CHECK: [[label_bRet_cmp_false3:.*]] ; preds =
// CHECK:%[[RET3:.*]] = load i1, i1* %[[bReturned]]
// CHECK:%[[NRET3:.*]] = icmp ne i1 %[[RET3]], false
// CHECK:br i1 %[[NRET3]],

  }
// CHECK: [[endif:.*]] ; preds =

// CHECK: [[forCond:.*]]; preds =


// CHECK: [[forBody:.*]] ; preds =
  for (int j=0;j<i;j++) {
    c += pow(2,j);
// CHECK: [[if_in_loop:.*]] ; preds =
    if (c > 10)
// set bIsReturn to true
// CHECK:store i1 true, i1* %[[bReturned]]
// dxBreak.
// CHECK:br i1 true,
      return -2;

// CHECK: [[endif_in_loop:.*]] ; preds =

// CHECK: [[for_inc:.*]] ; preds =

// Guard after loop.
// CHECK: [[label_bRet_cmp_false5:.*]] ; preds =
// CHECK:%[[RET6:.*]] = load i1, i1* %[[bReturned]]
// CHECK:%[[NRET6:.*]] = icmp ne i1 %[[RET6]], false
// CHECK:br i1 %[[NRET6]],

  }
// CHECK: [[for_end:.*]] ; preds =
// CHECK:switch i32
  switch (i) {
// CHECK: [[case1:.*]] ; preds =
    case 1:
     c += log(a.x);
     break;

// CHECK: [[case2:.*]] ; preds =
    case 2:

        c += cos(a.y);
     break;

// CHECK: [[case3:.*]] ; preds =
    case 3:

// CHECK: [[if_in_switch:.*]]  ; preds =
         if (c < 10)
// set bIsReturn to true
// CHECK:store i1 true, i1* %[[bReturned]]
// return just change to branch out of switch.
         return -3;

// CHECK: [[endif_in_switch:.*]] ; preds =
       c += sin(a.x);
     break;
  }
// guard code after switch.
// CHECK: [[label_bRet_cmp_false6:.*]] ; preds =
// CHECK:%[[RET8:.*]] = load i1, i1* %[[bReturned]]
// CHECK:%[[NRET8:.*]] = icmp ne i1 %[[RET8]], false
// CHECK:br i1 %[[NRET8]]

// CHECK: [[end_switch:.*]]; preds =

// CHECK: [[return:.*]] ; preds =
// CHECK-NOT:preds
// CHECK:ret

  return c;
}