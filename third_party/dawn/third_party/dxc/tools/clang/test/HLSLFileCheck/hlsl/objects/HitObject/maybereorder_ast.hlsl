// RUN: %dxc -T lib_6_9 -E main %s -ast-dump-implicit | FileCheck %s

// CHECK: |-FunctionDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit used MaybeReorderThread 'void (dx::HitObject)' extern
// CHECK-NEXT: | |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> HitObject 'dx::HitObject':'dx::HitObject'
// CHECK-NEXT: | |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 359
// CHECK-NEXT: | `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""

// CHECK: |-FunctionDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit used MaybeReorderThread 'void (dx::HitObject, unsigned int, unsigned int)' extern
// CHECK-NEXT: | |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> HitObject 'dx::HitObject':'dx::HitObject'
// CHECK-NEXT: | |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> CoherenceHint 'unsigned int'
// CHECK-NEXT: | |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> NumCoherenceHintBitsFromLSB 'unsigned int'
// CHECK-NEXT: | |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 359
// CHECK-NEXT: | `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""

// CHECK: `-FunctionDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit used MaybeReorderThread 'void (unsigned int, unsigned int)' extern
// CHECK-NEXT:   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> CoherenceHint 'unsigned int'
// CHECK-NEXT:   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> NumCoherenceHintBitsFromLSB 'unsigned int'
// CHECK-NEXT:   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 359
// CHECK-NEXT:   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""


[shader("raygeneration")]
void main() {
  dx::HitObject hit;
  dx::MaybeReorderThread(hit);
  dx::MaybeReorderThread(hit, 0xf1, 3);
  dx::MaybeReorderThread(0xf2, 7);
}