// RUN: %dxc -T lib_6_9 -E main %s | FileCheck %s --check-prefix DXIL
// RUN: %dxc -T lib_6_9 -E main %s -fcgl | FileCheck %s --check-prefix FCGL
// RUN: %dxc -T lib_6_9 -E main %s -ast-dump-implicit | FileCheck %s --check-prefix AST

// AST: |-FunctionDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit used MaybeReorderThread 'void (dx::HitObject)' extern
// AST-NEXT: | |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> HitObject 'dx::HitObject':'dx::HitObject'
// AST-NEXT: | |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 359
// AST-NEXT: | `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""

// AST: |-FunctionDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit used MaybeReorderThread 'void (dx::HitObject, unsigned int, unsigned int)' extern
// AST-NEXT: | |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> HitObject 'dx::HitObject':'dx::HitObject'
// AST-NEXT: | |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> CoherenceHint 'unsigned int'
// AST-NEXT: | |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> NumCoherenceHintBitsFromLSB 'unsigned int'
// AST-NEXT: | |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 359
// AST-NEXT: | `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""

// AST: `-FunctionDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit used MaybeReorderThread 'void (unsigned int, unsigned int)' extern
// AST-NEXT:   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> CoherenceHint 'unsigned int'
// AST-NEXT:   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> NumCoherenceHintBitsFromLSB 'unsigned int'
// AST-NEXT:   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 359
// AST-NEXT:   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""

// FCGL: call void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32 359, %dx.types.HitObject* %[[NOP:[^ ]+]])
// FCGL-NEXT: call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32, i32)"(i32 359, %dx.types.HitObject* %[[NOP]], i32 241, i32 3)
// FCGL-NEXT: call void @"dx.hl.op..void (i32, i32, i32)"(i32 359, i32 242, i32 7)

// DXIL:  call void @dx.op.maybeReorderThread(i32 268, %dx.types.HitObject %[[NOP:[^ ]+]], i32 undef, i32 0)  ; MaybeReorderThread(hitObject,coherenceHint,numCoherenceHintBitsFromLSB)
// DXIL-NEXT:  call void @dx.op.maybeReorderThread(i32 268, %dx.types.HitObject %[[NOP]], i32 241, i32 3)  ; MaybeReorderThread(hitObject,coherenceHint,numCoherenceHintBitsFromLSB)
// DXIL-NEXT:  call void @dx.op.maybeReorderThread(i32 268, %dx.types.HitObject %[[NOP]], i32 242, i32 7)  ; MaybeReorderThread(hitObject,coherenceHint,numCoherenceHintBitsFromLSB)

[shader("raygeneration")]
void main() {
  dx::HitObject hit;
  dx::MaybeReorderThread(hit);
  dx::MaybeReorderThread(hit, 0xf1, 3);
  dx::MaybeReorderThread(0xf2, 7);
}
