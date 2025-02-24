SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupOr_ae58b6() {
  int res = asint(WaveActiveBitOr(asuint(int(1))));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupOr_ae58b6()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupOr_ae58b6()));
}

FXC validation failure:
<scrubbed_path>(4,19-49): error X3004: undeclared identifier 'WaveActiveBitOr'


tint executable returned error: exit status 1
