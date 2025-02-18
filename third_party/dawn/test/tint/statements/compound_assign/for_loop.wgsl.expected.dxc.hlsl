[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer v : register(u0);
static uint i = 0u;

int idx1() {
  i = (i + 1u);
  return 1;
}

int idx2() {
  i = (i + 2u);
  return 1;
}

int idx3() {
  i = (i + 3u);
  return 1;
}

void foo() {
  float a[4] = (float[4])0;
  {
    int tint_symbol_save = idx1();
    a[min(uint(tint_symbol_save), 3u)] = (a[min(uint(tint_symbol_save), 3u)] * 2.0f);
    while (true) {
      int tint_symbol_2 = idx2();
      if (!((a[min(uint(tint_symbol_2), 3u)] < 10.0f))) {
        break;
      }
      {
      }
      {
        int tint_symbol_1_save = idx3();
        a[min(uint(tint_symbol_1_save), 3u)] = (a[min(uint(tint_symbol_1_save), 3u)] + 1.0f);
      }
    }
  }
}
