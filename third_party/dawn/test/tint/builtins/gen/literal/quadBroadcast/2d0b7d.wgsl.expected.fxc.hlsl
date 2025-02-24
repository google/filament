SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint4 quadBroadcast_2d0b7d() {
  uint4 res = QuadReadLaneAt((1u).xxxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_2d0b7d()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_2d0b7d()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-43): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
