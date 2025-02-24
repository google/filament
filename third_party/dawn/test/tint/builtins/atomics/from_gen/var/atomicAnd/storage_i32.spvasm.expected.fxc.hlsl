//
// fragment_main
//
RWByteAddressBuffer sb_rw : register(u0);

int sb_rwatomicAnd(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedAnd(offset, value, original_value);
  return original_value;
}


void atomicAnd_152966() {
  int arg_1 = 0;
  int res = 0;
  arg_1 = 1;
  int x_20 = arg_1;
  int x_13 = sb_rwatomicAnd(0u, x_20);
  res = x_13;
  return;
}

void fragment_main_1() {
  atomicAnd_152966();
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

int sb_rwatomicAnd(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedAnd(offset, value, original_value);
  return original_value;
}


void atomicAnd_152966() {
  int arg_1 = 0;
  int res = 0;
  arg_1 = 1;
  int x_20 = arg_1;
  int x_13 = sb_rwatomicAnd(0u, x_20);
  res = x_13;
  return;
}

void compute_main_1() {
  atomicAnd_152966();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
