//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupXor_7750d6() {
  uint res = WaveActiveBitXor(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupXor_7750d6());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupXor_7750d6() {
  uint res = WaveActiveBitXor(1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupXor_7750d6());
}

