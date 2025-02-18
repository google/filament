SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupAll_c962bd() {
  bool arg_0 = true;
  bool res = WaveActiveAllTrue(arg_0);
  return ((all((res == false))) ? (int(1)) : (int(0)));
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAll_c962bd()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAll_c962bd()));
}

FXC validation failure:
<scrubbed_path>(5,14-37): error X3004: undeclared identifier 'WaveActiveAllTrue'


tint executable returned error: exit status 1
