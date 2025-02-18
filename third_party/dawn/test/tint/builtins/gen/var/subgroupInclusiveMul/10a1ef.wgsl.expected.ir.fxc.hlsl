SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float16_t subgroupInclusiveMul_10a1ef() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t v = arg_0;
  float16_t res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupInclusiveMul_10a1ef());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupInclusiveMul_10a1ef());
}

FXC validation failure:
<scrubbed_path>(3,1-9): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
