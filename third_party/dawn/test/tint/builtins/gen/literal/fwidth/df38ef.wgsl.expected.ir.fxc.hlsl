
RWByteAddressBuffer prevent_dce : register(u0);
float fwidth_df38ef() {
  float res = fwidth(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(fwidth_df38ef()));
}

