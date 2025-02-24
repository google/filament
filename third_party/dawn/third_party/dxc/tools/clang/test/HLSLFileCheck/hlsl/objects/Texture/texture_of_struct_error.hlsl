// RUN: %dxc -T ps_6_0 -E main %s | FileCheck %s
// CHECK: error: texture resource texel type must be scalar or vector
struct Struct { float f; };
Texture2D<Struct> t;
RWTexture2D<Struct> rwt;
SamplerState s;
float main(float2 f2 : F2, int2 i2 : I) : SV_TARGET
{
    // Ensure semantic analysis doesn't crash
    rwt[i2] = t.Load(int3(i2, 0));
    t.Gather(s, f2, i2); // Test template resolution with INTRIN_COMPTYPE_FROM_TYPE_ELT0
    return 0;
}