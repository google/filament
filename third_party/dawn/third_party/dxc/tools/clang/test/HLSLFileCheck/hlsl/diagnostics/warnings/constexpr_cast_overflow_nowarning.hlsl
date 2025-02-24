// RUN: %dxc -E real_lit_to_flt_nowarning -T vs_6_0 %s | FileCheck %s
// RUN: %dxc -E real_lit_to_half_nowarning -T vs_6_0 %s | FileCheck %s
// RUN: %dxc -E int_lit_to_half_nowarning -T vs_6_0 %s | FileCheck %s
// RUN: %dxc -E real_lit_to_int_nowarning -T vs_6_0 %s | FileCheck %s
// CHECK-NOT: warning: overflow in expression when converting to

// Verify that when a constant is cast to a different type falls with in the
// the valid range of destination type, then no overflow warning is reported.

float real_lit_to_flt_nowarning() {
  return 3.4e10;
}

min16float real_lit_to_half_nowarning() {
  return 65500.0;
}

min16float int_lit_to_half_nowarning() {
  return 65500;
}

int real_lit_to_int_nowarning() {  
  return 3.4;
}