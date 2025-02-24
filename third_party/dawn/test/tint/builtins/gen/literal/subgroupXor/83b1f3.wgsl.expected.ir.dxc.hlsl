//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupXor_83b1f3() {
  int4 arg = (int(1)).xxxx;
  int4 res = asint(WaveActiveBitXor(asuint(arg)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupXor_83b1f3()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupXor_83b1f3() {
  int4 arg = (int(1)).xxxx;
  int4 res = asint(WaveActiveBitXor(asuint(arg)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupXor_83b1f3()));
}

