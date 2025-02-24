//
// fragment_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicXor_c1b78c() {
  int res = int(0);
  int v = int(0);
  sb_rw.InterlockedXor(int(0u), int(1), v);
  int x_9 = v;
  res = x_9;
}

void fragment_main_1() {
  atomicXor_c1b78c();
}

void fragment_main() {
  fragment_main_1();
}

//
// compute_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicXor_c1b78c() {
  int res = int(0);
  int v = int(0);
  sb_rw.InterlockedXor(int(0u), int(1), v);
  int x_9 = v;
  res = x_9;
}

void compute_main_1() {
  atomicXor_c1b78c();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

