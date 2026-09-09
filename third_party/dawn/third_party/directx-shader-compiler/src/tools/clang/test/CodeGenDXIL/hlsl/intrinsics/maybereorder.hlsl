// REQUIRES: dxil-1-9
// RUN: %dxc -T lib_6_9 -E main %s | FileCheck %s --check-prefix DXIL
// RUN: %dxc -T lib_6_9 -E main %s -fcgl | FileCheck %s --check-prefix FCGL

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
