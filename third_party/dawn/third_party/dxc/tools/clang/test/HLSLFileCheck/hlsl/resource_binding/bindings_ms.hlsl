// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// Ensure that MS texture bindings are reported correctly

// CHECK: ; Resource Bindings:
// CHECK: ;
// CHECK: ; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
// CHECK: ; ------------------------------ ---------- ------- ----------- ------- -------------- ------
// CHECK: ; Tex1                              texture     f32        2dMS      T0             t0     1
// CHECK: ; Tex2                              texture     f32        2dMS      T1             t1     4
// CHECK: ; Tex3                              texture     f32        2dMS      T2             t5unbounded
// CHECK: ; Tex4                              texture     f32       2dMS8      T3      t0,space1     1
// CHECK: ; Tex5                              texture     f32       2dMS8      T4      t1,space1     4
// CHECK: ; Tex6                              texture     f32       2dMS8      T5      t5,space1unbounded
// CHECK: ; TexArr1                           texture     f32   2darrayMS      T6      t0,space2     1
// CHECK: ; TexArr2                           texture     f32   2darrayMS      T7      t1,space2     4
// CHECK: ; TexArr3                           texture     f32   2darrayMS      T8      t5,space2unbounded
// CHECK: ; TexArr4                           texture     f32  2darrayMS8      T9      t0,space3     1
// CHECK: ; TexArr5                           texture     f32  2darrayMS8     T10      t1,space3     4
// CHECK: ; TexArr6                           texture     f32  2darrayMS8     T11      t5,space3unbounded
// CHECK: ; Tex7                              texture     f32        2dMS     T12      t0,space4unbounded
// CHECK: ; Tex8                              texture     f32       2dMS8     T13      t0,space5unbounded
// CHECK: ; TexArr7                           texture     f32   2darrayMS     T14      t0,space6unbounded
// CHECK: ; TexArr8                           texture     f32  2darrayMS8     T15      t0,space7unbounded

Texture2DMS<float4> Tex1 : register(t0);
Texture2DMS<float4> Tex2[4] : register(t1);
Texture2DMS<float4> Tex3[] : register(t5);  // unbounded
Texture2DMS<float4,8> Tex4 : register(t0, space1);
Texture2DMS<float4,8> Tex5[4] : register(t1, space1);
Texture2DMS<float4,8> Tex6[] : register(t5, space1);  // unbounded

Texture2DMSArray<float4> TexArr1 : register(t0, space2);
Texture2DMSArray<float4> TexArr2[4] : register(t1, space2);
Texture2DMSArray<float4> TexArr3[] : register(t5, space2);  // unbounded
Texture2DMSArray<float4,8> TexArr4 : register(t0, space3);
Texture2DMSArray<float4,8> TexArr5[4] : register(t1, space3);
Texture2DMSArray<float4,8> TexArr6[] : register(t5, space3);  // unbounded

Texture2DMS<float4> Tex7[][3] : register(t0, space4);  // unbounded
Texture2DMS<float4,8> Tex8[][3] : register(t0, space5);  // unbounded
Texture2DMSArray<float4> TexArr7[][4] : register(t0, space6);  // unbounded
Texture2DMSArray<float4,8> TexArr8[][5] : register(t0, space7);  // unbounded


float4 main(int4 a : A, float4 coord : TEXCOORD) : SV_TARGET
{
  return (float4)1.0
    * Tex1.Load(a.xy, a.w)
    * Tex2[a.w].sample[a.w][a.xy]
    * Tex3[18].Load(a.xy, 0)
    * Tex4.Load(a.xy, a.w)
    * Tex5[a.w].sample[a.w][a.xy]
    * Tex6[18].Load(a.xy, 0)
    * TexArr1.Load(a.xyz, a.w)
    * TexArr2[a.w].sample[a.w][a.xyz]
    * TexArr3[18].Load(a.xyz, 0)
    * TexArr4.Load(a.xyz, a.w)
    * TexArr5[a.w].sample[a.w][a.xyz]
    * TexArr6[18].Load(a.xyz, 0)
    * Tex7[19][1].Load(a.xy, 0)
    * Tex8[19][1].Load(a.xy, 0)
    * TexArr7[19][1].Load(a.xyz, 0)
    * TexArr8[19][1].Load(a.xyz, 0)
    ;
}
