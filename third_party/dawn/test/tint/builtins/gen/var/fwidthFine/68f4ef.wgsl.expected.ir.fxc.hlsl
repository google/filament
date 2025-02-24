
RWByteAddressBuffer prevent_dce : register(u0);
float4 fwidthFine_68f4ef() {
  float4 arg_0 = (1.0f).xxxx;
  float4 v = arg_0;
  float4 v_1 = ddx_fine(v);
  float4 res = (abs(v_1) + abs(ddy_fine(v)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(fwidthFine_68f4ef()));
}

