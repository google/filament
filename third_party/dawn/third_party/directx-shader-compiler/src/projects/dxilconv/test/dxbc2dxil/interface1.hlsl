// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




interface IFace
{
  float foo();
};

class First : IFace
{
  float f;
  SamplerState samp;
  Texture2D<float4> tex;
  //RWStructuredBuffer<float4> rwbuf; // not allowed
  float foo() {
    float4 result = tex.Sample(samp, float2(0.25, 0.75));
    //rwbuf[1] = result;
    return result.x * f;
  }
};

class Second : IFace
{
  float f;
  SamplerState samp;
  Texture2D<float4> tex;
  float foo() {
    float4 result = tex.Sample(samp, float2(0.125, 0.875));
    return result.x + f;
  }
};

interface IFace I[253];
uint i;

float main() : SV_TARGET
{
  return I[i].foo();
}
