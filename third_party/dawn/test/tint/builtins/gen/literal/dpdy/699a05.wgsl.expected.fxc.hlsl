RWByteAddressBuffer prevent_dce : register(u0);

float4 dpdy_699a05() {
  float4 res = ddy((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(dpdy_699a05()));
  return;
}
