// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// Make sure entry function exist.
// CHECK: @entry(

// Make sure function decl exist.
// CHECK: LoadInputMat
// CHECK: RotateMat
// CHECK: StoreOutputMat

// Make sure function props exist.
// CHECK: !dx.entryPoints = !{{{.*}}, {{.*}}}

// Make sure function props is correct for [numthreads(8,8,1)].
// CHECK: @entry,
// CHECK: !{i32 8, i32 8, i32 1}

cbuffer A {
  float a;
}

void StoreOutputMat(float2x2  m, uint gidx);
float2x2 LoadInputMat(uint x, uint y);
float2x2 RotateMat(float2x2 m, uint x, uint y);

[numthreads(8,8,1)]
void entry( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
    float2x2 f2x2 = LoadInputMat(gid.x, gid.y);

    f2x2 = RotateMat(f2x2, tid.x, tid.y) + a;

    StoreOutputMat(f2x2, gidx);
}
