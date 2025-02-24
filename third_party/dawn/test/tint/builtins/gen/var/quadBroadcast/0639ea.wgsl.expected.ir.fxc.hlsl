SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int quadBroadcast_0639ea() {
  int arg_0 = int(1);
  int res = QuadReadLaneAt(arg_0, 1u);
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
<scrubbed_path>(5,13-37): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
