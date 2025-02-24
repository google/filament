//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupXor_468721() {
  uint3 res = WaveActiveBitXor((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupXor_468721());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupXor_468721() {
  uint3 res = WaveActiveBitXor((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupXor_468721());
}

