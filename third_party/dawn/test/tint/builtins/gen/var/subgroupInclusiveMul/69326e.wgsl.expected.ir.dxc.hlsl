//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupInclusiveMul_69326e() {
  float4 arg_0 = (1.0f).xxxx;
  float4 v = arg_0;
  float4 res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveMul_69326e()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupInclusiveMul_69326e() {
  float4 arg_0 = (1.0f).xxxx;
  float4 v = arg_0;
  float4 res = (WavePrefixProduct(v) * v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveMul_69326e()));
}

