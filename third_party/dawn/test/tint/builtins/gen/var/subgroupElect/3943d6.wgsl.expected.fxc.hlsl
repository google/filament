SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupElect_3943d6() {
  bool res = WaveIsFirstLane();
  return (all((res == false)) ? 1 : 0);
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupElect_3943d6()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupElect_3943d6()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-30): error X3004: undeclared identifier 'WaveIsFirstLane'


tint executable returned error: exit status 1
