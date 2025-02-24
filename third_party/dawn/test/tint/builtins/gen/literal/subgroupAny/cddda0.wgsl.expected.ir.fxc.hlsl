SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupAny_cddda0() {
  bool res = WaveActiveAnyTrue(true);
  return ((all((res == false))) ? (int(1)) : (int(0)));
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAny_cddda0()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAny_cddda0()));
}

FXC validation failure:
<scrubbed_path>(4,14-36): error X3004: undeclared identifier 'WaveActiveAnyTrue'


tint executable returned error: exit status 1
