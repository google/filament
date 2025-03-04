// RUN: %dxc -T ps_6_0 -E main %s | FileCheck %s
// Note: FXC accepts this
// CHECK: error: texture resource texel type must be scalar or vector
Texture2D<float1x1> t;
RWTexture2D<float1x1> rwt;
SamplerState s;
float main(float2 f2 : F2, int2 i2 : I) : SV_TARGET
{
    // Ensure semantic analysis doesn't crash
    rwt[i2] = t.Load(int3(i2, 0));
    t.Gather(s, f2, i2); // Test template resolution with INTRIN_COMPTYPE_FROM_TYPE_ELT0
    return 0;
}