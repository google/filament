SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint quadBroadcast_e6d948() {
  uint res = QuadReadLaneAt(1u, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_e6d948()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_e6d948()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-35): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
