// RUN: %dxc -T lib_6_9 -E main %s -Od | FileCheck %s --check-prefix DXIL

// DXIL: %[[HOA:[^ ]+]] = alloca %dx.types.HitObject, align 4
// DXIL-NEXT: %[[NOP:[^ ]+]] = call %dx.types.HitObject @dx.op.hitObject_MakeNop(i32 266)  ; HitObject_MakeNop()
// DXIL-NEXT: store %dx.types.HitObject %[[NOP]], %dx.types.HitObject* %[[HOA]]
// DXIL-NEXT: %[[LD0:[^ ]+]] = load %dx.types.HitObject, %dx.types.HitObject* %[[HOA]]
// DXIL-NEXT: call void @dx.op.maybeReorderThread(i32 268, %dx.types.HitObject %[[LD0]], i32 undef, i32 0)  ; MaybeReorderThread(hitObject,coherenceHint,numCoherenceHintBitsFromLSB)
// DXIL-NEXT: %[[LD1:[^ ]+]] = load %dx.types.HitObject, %dx.types.HitObject* %[[HOA]]
// DXIL-NEXT: call void @dx.op.maybeReorderThread(i32 268, %dx.types.HitObject %[[LD1]], i32 241, i32 3)  ; MaybeReorderThread(hitObject,coherenceHint,numCoherenceHintBitsFromLSB)
// DXIL-NEXT: %[[NOP2:[^ ]+]] = call %dx.types.HitObject @dx.op.hitObject_MakeNop(i32 266)  ; HitObject_MakeNop()
// DXIL-NEXT: call void @dx.op.maybeReorderThread(i32 268, %dx.types.HitObject %[[NOP2]], i32 242, i32 7)  ; MaybeReorderThread(hitObject,coherenceHint,numCoherenceHintBitsFromLSB)

[shader("raygeneration")]
void main() {
  dx::HitObject hit;
  dx::MaybeReorderThread(hit);
  dx::MaybeReorderThread(hit, 0xf1, 3);
  dx::MaybeReorderThread(0xf2, 7);
}
