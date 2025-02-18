//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupOr_ae58b6() {
  int res = asint(WaveActiveBitOr(asuint(1)));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupOr_ae58b6()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupOr_ae58b6() {
  int res = asint(WaveActiveBitOr(asuint(1)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupOr_ae58b6()));
  return;
}
