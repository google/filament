
RWTexture2D<float4> tex : register(u0);
void fragment_main() {
  float4 value = float4(1.0f, 2.0f, 3.0f, 4.0f);
  tex[int2(int(9), int(8))] = value;
}

