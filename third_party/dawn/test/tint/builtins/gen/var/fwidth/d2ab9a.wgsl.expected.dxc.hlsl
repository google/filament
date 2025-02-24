RWByteAddressBuffer prevent_dce : register(u0);

float4 fwidth_d2ab9a() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = fwidth(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(fwidth_d2ab9a()));
  return;
}
