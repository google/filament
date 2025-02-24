SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint quadBroadcast_e6d948() {
  uint arg_0 = 1u;
  uint res = QuadReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, quadBroadcast_e6d948());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, quadBroadcast_e6d948());
}

FXC validation failure:
<scrubbed_path>(5,14-38): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
