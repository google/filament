//
// fragment_main
//
RWByteAddressBuffer sb_rw : register(u0);

int sb_rwatomicMax(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedMax(offset, value, original_value);
  return original_value;
}


void atomicMax_92aa72() {
  int res = 0;
  int x_9 = sb_rwatomicMax(0u, 1);
  res = x_9;
  return;
}

void fragment_main_1() {
  atomicMax_92aa72();
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

int sb_rwatomicMax(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedMax(offset, value, original_value);
  return original_value;
}


void atomicMax_92aa72() {
  int res = 0;
  int x_9 = sb_rwatomicMax(0u, 1);
  res = x_9;
  return;
}

void compute_main_1() {
  atomicMax_92aa72();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
