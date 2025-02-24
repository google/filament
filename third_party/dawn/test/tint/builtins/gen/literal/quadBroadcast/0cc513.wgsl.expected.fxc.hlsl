SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float3 quadBroadcast_0cc513() {
  float3 res = QuadReadLaneAt((1.0f).xxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_0cc513()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_0cc513()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,16-45): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
