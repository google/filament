
RWByteAddressBuffer prevent_dce : register(u0);
float2 fwidth_b83ebb() {
  float2 arg_0 = (1.0f).xx;
  float2 res = fwidth(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(fwidth_b83ebb()));
}

