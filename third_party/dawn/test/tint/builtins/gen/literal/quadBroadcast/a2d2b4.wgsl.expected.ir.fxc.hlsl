SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint quadBroadcast_a2d2b4() {
  uint res = QuadReadLaneAt(1u, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, quadBroadcast_a2d2b4());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, quadBroadcast_a2d2b4());
}

FXC validation failure:
<scrubbed_path>(4,14-39): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
