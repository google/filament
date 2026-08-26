// RUN: %dxc -T lib_6_9 -E main %s -DMOD= | FileCheck %s --check-prefix IN
// RUN: %dxc -T lib_6_9 -E main %s -DMOD=in | FileCheck %s --check-prefix IN
// RUN: %dxc -T lib_6_9 -E main %s -DMOD=out | FileCheck %s --check-prefix OUT

// IN: %[[MISS:[^ ]+]] = call %dx.types.HitObject @dx.op.hitObject_MakeMiss(i32 265, i32 0, i32 0, float undef, float undef, float undef, float undef, float undef, float undef, float undef, float undef)  ; HitObject_MakeMiss(RayFlags,MissShaderIndex,Origin_X,Origin_Y,Origin_Z,TMin,Direction_X,Direction_Y,Direction_Z,TMax)
// IN: %[[ISNOP:[^ ]+]] = call i1 @dx.op.hitObject_StateScalar.i1(i32 271, %dx.types.HitObject %[[MISS]])  ; HitObject_IsNop(hitObject)
// IN: %[[ISNOP32:[^ ]+]] = zext i1 %[[ISNOP]] to i32
// IN: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{[^ ]+}}, i32 0, i32 undef, i32 %[[ISNOP32]]
// IN: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{[^ ]+}}, i32 1, i32 undef, i32 %[[ISNOP32]]
// IN-NOT: @dx.op.hitObject_MakeNop

// OUT: %[[NOP:[^ ]+]] = call %dx.types.HitObject @dx.op.hitObject_MakeNop(i32 266)  ; HitObject_MakeNop()
// OUT: %[[ISNOP:[^ ]+]] = call i1 @dx.op.hitObject_StateScalar.i1(i32 271, %dx.types.HitObject %[[NOP]])  ; HitObject_IsNop(hitObject)
// OUT: %[[ISNOP32:[^ ]+]] = zext i1 %[[ISNOP]] to i32
// OUT: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{[^ ]+}}, i32 0, i32 undef, i32 %[[ISNOP32]]
// OUT: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{[^ ]+}}, i32 1, i32 undef, i32 %[[ISNOP32]]
// OUT-NOT: @dx.op.hitObject_MakeMiss

RWBuffer<uint> output : register(u0, space0);

// in, out, inout and no modifier all have the same behaviour
void MakeNop(MOD dx::HitObject obj [2]) {
  obj[0] = dx::HitObject::MakeNop();
  obj[1] = dx::HitObject::MakeNop();
}

[shader("raygeneration")]
void RayGen()
{
  RayDesc ray;
  dx::HitObject obj[2] = {dx::HitObject::MakeMiss(0, 0, ray), dx::HitObject::MakeMiss(0, 0, ray)};

  MakeNop(obj);
  output[0] = obj[0].IsNop();
  output[1] = obj[1].IsNop();
}
