SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 quadBroadcast_f60448() {
  uint2 res = QuadReadLaneAt((1u).xx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, quadBroadcast_f60448());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, quadBroadcast_f60448());
}

FXC validation failure:
<scrubbed_path>(4,15-41): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
