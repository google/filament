//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupXor_694b17() {
  int arg_0 = 1;
  int res = asint(WaveActiveBitXor(asuint(arg_0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_694b17()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupXor_694b17() {
  int arg_0 = 1;
  int res = asint(WaveActiveBitXor(asuint(arg_0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_694b17()));
  return;
}
