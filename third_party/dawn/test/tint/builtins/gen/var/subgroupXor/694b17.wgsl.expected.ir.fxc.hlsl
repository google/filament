SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupXor_694b17() {
  int arg_0 = int(1);
  int res = asint(WaveActiveBitXor(asuint(arg_0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_694b17()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_694b17()));
}

FXC validation failure:
<scrubbed_path>(5,19-49): error X3004: undeclared identifier 'WaveActiveBitXor'


tint executable returned error: exit status 1
