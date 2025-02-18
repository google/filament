
RWByteAddressBuffer prevent_dce : register(u0);
float4 fwidth_d2ab9a() {
  float4 res = fwidth((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(fwidth_d2ab9a()));
}

