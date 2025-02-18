//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

int sb_rwatomicAdd(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedAdd(offset, value, original_value);
  return original_value;
}


int atomicAdd_d32fe4() {
  int res = sb_rwatomicAdd(0u, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicAdd_d32fe4()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

int sb_rwatomicAdd(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedAdd(offset, value, original_value);
  return original_value;
}


int atomicAdd_d32fe4() {
  int res = sb_rwatomicAdd(0u, 1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicAdd_d32fe4()));
  return;
}
