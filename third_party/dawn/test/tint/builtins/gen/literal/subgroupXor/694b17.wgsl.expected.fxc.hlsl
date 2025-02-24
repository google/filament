SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupXor_694b17() {
  int res = asint(WaveActiveBitXor(asuint(1)));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_694b17()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_694b17()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,19-45): error X3004: undeclared identifier 'WaveActiveBitXor'


tint executable returned error: exit status 1
