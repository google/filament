//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<uint4> arg_0 : register(u0, space1);
uint textureDimensions_7228de() {
  uint v = 0u;
  arg_0.GetDimensions(v);
  uint res = v;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, textureDimensions_7228de());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<uint4> arg_0 : register(u0, space1);
uint textureDimensions_7228de() {
  uint v = 0u;
  arg_0.GetDimensions(v);
  uint res = v;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, textureDimensions_7228de());
}

