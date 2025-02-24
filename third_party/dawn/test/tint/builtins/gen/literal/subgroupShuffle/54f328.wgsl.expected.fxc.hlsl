SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupShuffle_54f328() {
  uint res = WaveReadLaneAt(1u, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_54f328()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_54f328()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-34): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
