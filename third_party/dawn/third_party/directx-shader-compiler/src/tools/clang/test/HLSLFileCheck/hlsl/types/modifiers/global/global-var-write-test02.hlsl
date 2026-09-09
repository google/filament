// RUN: %dxc -E main -T ps_6_0 /Gec -HV 2016  %s | FileCheck %s

// Modifying local const should fail even with HV <= 2016
// CHECK: warning: /Gec flag is a deprecated functionality.
// CHECK: error: cannot assign to variable 'l_s' with const-qualified type 'const int'
// CHECK: note: variable 'l_s' declared const here

float main(uint a
           : A) : SV_Target {
  // update global scalar
  const int l_s = 1;
  l_s *= 2;
  return 1.33f * l_s;
}