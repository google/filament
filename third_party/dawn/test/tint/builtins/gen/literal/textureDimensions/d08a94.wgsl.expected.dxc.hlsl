//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<int4> arg_0 : register(u0, space1);

uint textureDimensions_d08a94() {
  uint tint_tmp;
  arg_0.GetDimensions(tint_tmp);
  uint res = tint_tmp;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureDimensions_d08a94()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<int4> arg_0 : register(u0, space1);

uint textureDimensions_d08a94() {
  uint tint_tmp;
  arg_0.GetDimensions(tint_tmp);
  uint res = tint_tmp;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(textureDimensions_d08a94()));
  return;
}
