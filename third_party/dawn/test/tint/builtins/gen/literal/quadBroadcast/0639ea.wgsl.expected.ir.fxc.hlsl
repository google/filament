SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int quadBroadcast_0639ea() {
  int res = QuadReadLaneAt(int(1), 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_0639ea()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_0639ea()));
}

FXC validation failure:
<scrubbed_path>(4,13-38): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
