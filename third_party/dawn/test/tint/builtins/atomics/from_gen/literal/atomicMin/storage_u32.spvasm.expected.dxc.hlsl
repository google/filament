//
// fragment_main
//
RWByteAddressBuffer sb_rw : register(u0);

uint sb_rwatomicMin(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedMin(offset, value, original_value);
  return original_value;
}


void atomicMin_c67a74() {
  uint res = 0u;
  uint x_9 = sb_rwatomicMin(0u, 1u);
  res = x_9;
  return;
}

void fragment_main_1() {
  atomicMin_c67a74();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}
//
// compute_main
//
RWByteAddressBuffer sb_rw : register(u0);

uint sb_rwatomicMin(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedMin(offset, value, original_value);
  return original_value;
}


void atomicMin_c67a74() {
  uint res = 0u;
  uint x_9 = sb_rwatomicMin(0u, 1u);
  res = x_9;
  return;
}

void compute_main_1() {
  atomicMin_c67a74();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
