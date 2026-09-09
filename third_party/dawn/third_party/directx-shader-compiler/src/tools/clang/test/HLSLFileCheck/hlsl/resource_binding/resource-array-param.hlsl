// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
Texture2D Tex4[4];
Texture2D Tex[];

float4 lookup(Texture2D tex[], int3 coord) {
  return tex[0].Load(coord);
}

float4 lookup4(Texture2D tex[4], int3 coord) {
  return tex[0].Load(coord);
}

float4 main() : SV_Target
{
  return lookup(Tex, 0) + lookup(Tex4, 1) + lookup4(Tex, 2) + lookup4(Tex4, 3);
}
