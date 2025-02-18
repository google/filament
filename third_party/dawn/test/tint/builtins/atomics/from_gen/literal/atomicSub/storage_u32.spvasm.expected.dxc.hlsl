//
// fragment_main
//
RWByteAddressBuffer sb_rw : register(u0);

uint sb_rwatomicSub(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAdd(offset, -value, original_value);
  return original_value;
}


void atomicSub_15bfc9() {
  uint res = 0u;
  uint x_9 = sb_rwatomicSub(0u, 1u);
  res = x_9;
  return;
}

void fragment_main_1() {
  atomicSub_15bfc9();
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

uint sb_rwatomicSub(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAdd(offset, -value, original_value);
  return original_value;
}


void atomicSub_15bfc9() {
  uint res = 0u;
  uint x_9 = sb_rwatomicSub(0u, 1u);
  res = x_9;
  return;
}

void compute_main_1() {
  atomicSub_15bfc9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
