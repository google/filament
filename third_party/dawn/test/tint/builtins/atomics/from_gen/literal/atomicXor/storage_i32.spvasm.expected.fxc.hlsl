//
// fragment_main
//
RWByteAddressBuffer sb_rw : register(u0);

int sb_rwatomicXor(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedXor(offset, value, original_value);
  return original_value;
}


void atomicXor_c1b78c() {
  int res = 0;
  int x_9 = sb_rwatomicXor(0u, 1);
  res = x_9;
  return;
}

void fragment_main_1() {
  atomicXor_c1b78c();
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

int sb_rwatomicXor(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedXor(offset, value, original_value);
  return original_value;
}


void atomicXor_c1b78c() {
  int res = 0;
  int x_9 = sb_rwatomicXor(0u, 1);
  res = x_9;
  return;
}

void compute_main_1() {
  atomicXor_c1b78c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
