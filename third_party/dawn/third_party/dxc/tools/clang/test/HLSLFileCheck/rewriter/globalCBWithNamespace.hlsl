// RUN: %dxr -decl-global-cb  -line-directive   %s | FileCheck %s

// Make sure namespace in global cb works.
// CHECK:cbuffer GlobalCB {
// CHECK-NEXT:namespace N {
// CHECK-NEXT:float a;
// CHECK-NEXT:}
// CHECK-NEXT:namespace N2 {
// CHECK-NEXT:float b;
// CHECK-NEXT:}
// CHECK-NEXT:}


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

