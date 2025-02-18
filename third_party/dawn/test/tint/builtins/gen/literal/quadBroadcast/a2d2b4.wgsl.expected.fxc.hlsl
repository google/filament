SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint quadBroadcast_a2d2b4() {
  uint res = QuadReadLaneAt(1u, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_a2d2b4()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_a2d2b4()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-34): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
