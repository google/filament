SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupOr_ae58b6() {
  int arg_0 = 1;
  int res = asint(WaveActiveBitOr(asuint(arg_0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupOr_ae58b6()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupOr_ae58b6()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,19-48): error X3004: undeclared identifier 'WaveActiveBitOr'


tint executable returned error: exit status 1
