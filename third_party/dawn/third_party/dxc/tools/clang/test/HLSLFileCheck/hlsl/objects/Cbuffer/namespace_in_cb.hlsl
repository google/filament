// RUN: %dxc -E main -T ps_6_0  %s  | FileCheck %s

// CHECK:cbuffer $Globals
// CHECK:struct $Globals
// CHECK:{
// CHECK:float N::a;                                   ; Offset:    0
// CHECK:float N2::b;                                  ; Offset:    4
// CHECK:} $Globals;                                       ; Offset:    0 Size:     8
// CHECK:}

// CHECK:cbuffer B
// CHECK:struct B
// CHECK:{
// CHECK:float N::c;                                   ; Offset:    0
// CHECK:float N2::d;                                  ; Offset:    4
// CHECK:} B;                                              ; Offset:    0 Size:     8

namespace N {
float a;
}

cbuffer B {
  namespace N {
  float c;
  }
  namespace N2 {
    float d;
  }
}

namespace N2 {
  float b;
}

float main() : SV_Target {
  return N::a + N::c + N2::d + N2::b;
}

