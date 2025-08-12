// REQUIRES: dxil-1-9
// RUN: %dxc -T lib_6_9 -E main %s | FileCheck %s --check-prefix DXIL

// DXIL: %dx.types.HitObject = type { i8* }

// DXIL:   %[[NOP:[^ ]+]] = call %dx.types.HitObject @dx.op.hitObject_MakeNop(i32 266)  ; HitObject_MakeNop()
// DXIL:   %[[HIT:[^ ]+]] = call %dx.types.HitObject @dx.op.hitObject_SetShaderTableIndex(i32 287, %dx.types.HitObject %[[NOP]], i32 1)  ; HitObject_SetShaderTableIndex(hitObject,shaderTableIndex)
// DXIL-DAG:   %{{[^ ]+}} = call i1 @dx.op.hitObject_StateScalar.i1(i32 270, %dx.types.HitObject %[[HIT]])  ; HitObject_IsHit(hitObject)
// DXIL-DAG:   %{{[^ ]+}} = call i1 @dx.op.hitObject_StateScalar.i1(i32 269, %dx.types.HitObject %[[HIT]])  ; HitObject_IsMiss(hitObject)
// DXIL-DAG:   %{{[^ ]+}} = call i1 @dx.op.hitObject_StateScalar.i1(i32 271, %dx.types.HitObject %[[HIT]])  ; HitObject_IsNop(hitObject)
// DXIL-DAG:   %{{[^ ]+}} = call i32 @dx.op.hitObject_StateScalar.i32(i32 281, %dx.types.HitObject %[[HIT]])  ; HitObject_GeometryIndex(hitObject)
// DXIL-DAG:   %{{[^ ]+}} = call i32 @dx.op.hitObject_StateScalar.i32(i32 285, %dx.types.HitObject %[[HIT]])  ; HitObject_HitKind(hitObject)
// DXIL-DAG:   %{{[^ ]+}} = call i32 @dx.op.hitObject_StateScalar.i32(i32 282, %dx.types.HitObject %[[HIT]])  ; HitObject_InstanceIndex(hitObject)
// DXIL-DAG:   %{{[^ ]+}} = call i32 @dx.op.hitObject_StateScalar.i32(i32 283, %dx.types.HitObject %[[HIT]])  ; HitObject_InstanceID(hitObject)
// DXIL-DAG:   %{{[^ ]+}} = call i32 @dx.op.hitObject_StateScalar.i32(i32 284, %dx.types.HitObject %[[HIT]])  ; HitObject_PrimitiveIndex(hitObject)
// DXIL-DAG:   %{{[^ ]+}} = call i32 @dx.op.hitObject_StateScalar.i32(i32 286, %dx.types.HitObject %[[HIT]])  ; HitObject_ShaderTableIndex(hitObject)
// DXIL-DAG:   %{{[^ ]+}} = call i32 @dx.op.hitObject_LoadLocalRootTableConstant(i32 288, %dx.types.HitObject %[[HIT]], i32 40)  ; HitObject_LoadLocalRootTableConstant(hitObject,offset)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 277, %dx.types.HitObject %[[HIT]], i32 0)  ; HitObject_ObjectRayOrigin(hitObject,component)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 277, %dx.types.HitObject %[[HIT]], i32 1)  ; HitObject_ObjectRayOrigin(hitObject,component)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 277, %dx.types.HitObject %[[HIT]], i32 2)  ; HitObject_ObjectRayOrigin(hitObject,component)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 278, %dx.types.HitObject %[[HIT]], i32 0)  ; HitObject_ObjectRayDirection(hitObject,component)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 278, %dx.types.HitObject %[[HIT]], i32 1)  ; HitObject_ObjectRayDirection(hitObject,component)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 278, %dx.types.HitObject %[[HIT]], i32 2)  ; HitObject_ObjectRayDirection(hitObject,component)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 275, %dx.types.HitObject %[[HIT]], i32 0)  ; HitObject_WorldRayOrigin(hitObject,component)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 275, %dx.types.HitObject %[[HIT]], i32 1)  ; HitObject_WorldRayOrigin(hitObject,component)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 275, %dx.types.HitObject %[[HIT]], i32 2)  ; HitObject_WorldRayOrigin(hitObject,component)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 276, %dx.types.HitObject %[[HIT]], i32 0)  ; HitObject_WorldRayDirection(hitObject,component)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 276, %dx.types.HitObject %[[HIT]], i32 1)  ; HitObject_WorldRayDirection(hitObject,component)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 276, %dx.types.HitObject %[[HIT]], i32 2)  ; HitObject_WorldRayDirection(hitObject,component)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[HIT]], i32 0, i32 0)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[HIT]], i32 0, i32 1)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[HIT]], i32 0, i32 2)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[HIT]], i32 0, i32 3)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[HIT]], i32 1, i32 0)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[HIT]], i32 1, i32 1)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[HIT]], i32 1, i32 2)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[HIT]], i32 1, i32 3)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[HIT]], i32 2, i32 0)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[HIT]], i32 2, i32 1)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[HIT]], i32 2, i32 2)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[HIT]], i32 2, i32 3)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[HIT]], i32 0, i32 0)  ; HitObject_WorldToObject3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[HIT]], i32 0, i32 1)  ; HitObject_WorldToObject3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[HIT]], i32 0, i32 2)  ; HitObject_WorldToObject3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[HIT]], i32 0, i32 3)  ; HitObject_WorldToObject3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[HIT]], i32 1, i32 0)  ; HitObject_WorldToObject3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[HIT]], i32 1, i32 1)  ; HitObject_WorldToObject3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[HIT]], i32 1, i32 2)  ; HitObject_WorldToObject3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[HIT]], i32 1, i32 3)  ; HitObject_WorldToObject3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[HIT]], i32 2, i32 0)  ; HitObject_WorldToObject3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[HIT]], i32 2, i32 1)  ; HitObject_WorldToObject3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[HIT]], i32 2, i32 2)  ; HitObject_WorldToObject3x4(hitObject,row,col)
// DXIL-DAG:   %{{[^ ]+}} = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[HIT]], i32 2, i32 3)  ; HitObject_WorldToObject3x4(hitObject,row,col)
// DXIL:   ret void

RWByteAddressBuffer outbuf;

template <int M, int N>
float hashM(in matrix<float, M, N> mat) {
  float h = 0.f;
  for (int i = 0; i < M; ++i)
    for (int j = 0; j < N; ++j)
      h += mat[i][j];
  return h;
}

[shader("raygeneration")]
void main() {
  dx::HitObject hit;
  int isum = 0;
  float fsum = 0.0f;
  vector<float, 3> vsum = 0;

  ///// Setters
  hit.SetShaderTableIndex(1);

  ///// Getters

  // i1 accessors
  isum += hit.IsHit();
  isum += hit.IsMiss();
  isum += hit.IsNop();

  // i32 accessors
  isum += hit.GetGeometryIndex();
  isum += hit.GetHitKind();
  isum += hit.GetInstanceIndex();
  isum += hit.GetInstanceID();
  isum += hit.GetPrimitiveIndex();
  isum += hit.GetShaderTableIndex();
  isum += hit.LoadLocalRootTableConstant(40);

  // float3 accessors
  vsum += hit.GetWorldRayOrigin();
  vsum += hit.GetWorldRayDirection();
  vsum += hit.GetObjectRayOrigin();
  vsum += hit.GetObjectRayDirection();
  fsum += vsum[0] + vsum[1] + vsum[2];

  // matrix accessors
  fsum += hashM<3, 4>(hit.GetObjectToWorld3x4());
  fsum += hashM<4, 3>(hit.GetObjectToWorld4x3());
  fsum += hashM<3, 4>(hit.GetWorldToObject3x4());
  fsum += hashM<4, 3>(hit.GetWorldToObject4x3());

  // f32 accessors
  isum += hit.GetRayFlags();
  fsum += hit.GetRayTMin();
  fsum += hit.GetRayTCurrent();

  outbuf.Store(0, fsum);
  outbuf.Store(4, isum);
}
