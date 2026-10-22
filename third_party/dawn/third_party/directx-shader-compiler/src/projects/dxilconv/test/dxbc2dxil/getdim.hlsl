// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




Texture2D<float4> tex1;
Texture2DArray<float4> tex2;
TextureCubeArray<float4> tex3;
Texture2DMSArray<float4> tex4;

RWTexture2D<float4> uav1;

float4 main(float4 a : A) : SV_Target
{
  float4 r = 0;
  uint MipLevel = a.w, Width = 0, Height = 0, Elements = 0, Depth = 0, NumberOfLevels = 0, NumberOfSamples = 0;
  tex1.GetDimensions(MipLevel, Width, Height, NumberOfLevels); r += MipLevel + Width + Height + Elements + Depth + NumberOfLevels + NumberOfSamples;
  tex1.GetDimensions(Width, Height); r += MipLevel + Width + Height + Elements + Depth + NumberOfLevels + NumberOfSamples;
  tex2.GetDimensions(MipLevel, Width, Height, Elements, NumberOfLevels); r += MipLevel + Width + Height + Elements + Depth + NumberOfLevels + NumberOfSamples;
  tex3.GetDimensions(MipLevel, Width, Height, Elements, NumberOfLevels); r += MipLevel + Width + Height + Elements + Depth + NumberOfLevels + NumberOfSamples;
  tex4.GetDimensions(Width, Height, Elements, NumberOfSamples); r += MipLevel + Width + Height + Elements + Depth + NumberOfLevels + NumberOfSamples;

  uav1.GetDimensions(Width, Height); r += MipLevel + Width + Height + Elements + Depth + NumberOfLevels + NumberOfSamples;

  return r;
}
