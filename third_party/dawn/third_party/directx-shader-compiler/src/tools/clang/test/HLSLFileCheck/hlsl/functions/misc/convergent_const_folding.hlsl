// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure dxil.convergent.marker will be const folding.
// CHECK:@dx.op.storeOutput

SamplerState ss1;
SamplerState ss0;

struct Option
{
 bool cond;
};


Texture2D<float4> Tex;

float4 ps(
 float2 uv,
 bool cond
)
{

 Option op;

 op.cond = cond;

 if (op.cond)
 {  
  float c = op.cond ? 0.0f : 1;
  uv = Tex.Sample(ss0, c).xy;
 }

 SamplerState texSampler = (op.cond?ss0:ss1);
 return Tex.Sample(texSampler, uv);
}

float4 main(float2 uv :UV) :SV_Target
{
  return ps(uv, 0);

}