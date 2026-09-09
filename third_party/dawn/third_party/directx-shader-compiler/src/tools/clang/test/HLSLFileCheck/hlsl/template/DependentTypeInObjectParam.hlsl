// RUN: %dxc -E main -T vs_6_2 -HV 2021 -enable-16bit-types -ast-dump %s | FileCheck -check-prefixes=CHECKAST %s
// RUN: %dxc -E main -T vs_6_2 -HV 2021 -enable-16bit-types %s | FileCheck -check-prefixes=CHECK %s

// CHECKAST: -CallExpr
// CHECKAST-NEXT: -ImplicitCastExpr
// CHECKAST-SAME: 'float (*)(Texture2D<vector<float, 4> >)' <FunctionToPointerDecay>

// CHECKAST: -CallExpr
// CHECKAST-NEXT: -ImplicitCastExpr
// CHECKAST-SAME: 'half (*)(Texture2D<vector<half, 4> >)' <FunctionToPointerDecay>

// CHECK: call %dx.types.ResRet.f32 @dx.op.textureLoad.f32
// CHECK: call %dx.types.ResRet.f16 @dx.op.textureLoad.f16

template<typename T>
T foo(Texture2D<vector<T, 4> > tex) {
  return tex[uint2(0,0)].x;
}

Texture2D<float4> Tex;
Texture2D<vector<float16_t, 4> > Tex2;

float main() : OUT {
  return foo(Tex) + foo(Tex2);
}
