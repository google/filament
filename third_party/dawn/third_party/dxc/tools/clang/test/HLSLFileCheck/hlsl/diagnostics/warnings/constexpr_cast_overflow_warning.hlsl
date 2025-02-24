// RUN: %dxc -E real_lit_to_flt_warning -T vs_6_0 %s | FileCheck -check-prefix=CHK-FLT %s
// RUN: %dxc -E real_lit_to_half_warning -T vs_6_0 %s | FileCheck -check-prefix=CHK-MINFLT1 %s
// RUN: %dxc -E int_lit_to_half_warning -T vs_6_0 %s | FileCheck -check-prefix=CHK-MINFLT2 %s
// RUN: %dxc -E real_lit_to_int_warning -T vs_6_0 %s | FileCheck -check-prefix=CHK-INT %s

// Verify that when a constant is cast to a different type leading to overflow
// a warning is generated notifying the same.

// CHK-FLT: warning: overflow in the expression when converting to 'float' type
float real_lit_to_flt_warning() {  
  return 3.4e50;
}

// CHK-MINFLT1: warning: overflow in the expression when converting to 'min16float' type
min16float real_lit_to_half_warning() {
  return 65520.0;
}

// CHK-MINFLT2: warning: overflow in the expression when converting to 'min16float' type
min16float int_lit_to_half_warning() {
  return 65520;
}

// CHK-INT: warning: overflow in the expression when converting to 'int' type
int real_lit_to_int_warning() {  
  return 3.4e20;
}