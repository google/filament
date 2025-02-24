// RUN: %dxc -E main -T ps_6_0 -HV 2018 %s | FileCheck %s

// CHECKNOT: bitcast
// CHECKNOT: i64 2
// CHECK: fcmp fast une
// CHECK: and
// CHECK: !"dx.controlflow.hints", i32 1

void CheckCond(float b, out bool cond) {
    cond = b != 1;
}

float main(float2 a : A, float b : B) : SV_Target
{
  bool cond;
  CheckCond(b, cond);
  [branch]
  if (cond && b > 2)
    return a.x>2? 1:2;
  else
    return a.y;
}
