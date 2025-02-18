[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer buffer : register(u0);
static uint v = 0u;

int idx1() {
  v = (v - 1u);
  return 1;
}

int idx2() {
  v = (v - 1u);
  return 2;
}

int idx3() {
  v = (v - 1u);
  return 3;
}

int idx4() {
  v = (v - 1u);
  return 4;
}

int idx5() {
  v = (v - 1u);
  return 0;
}

int idx6() {
  v = (v - 1u);
  return 2;
}

void main() {
  {
    uint tint_symbol_5 = 0u;
    buffer.GetDimensions(tint_symbol_5);
    uint tint_symbol_6 = (tint_symbol_5 / 64u);
    int tint_symbol_save = idx1();
    int tint_symbol_save_1 = idx2();
    int tint_symbol_1 = idx3();
    buffer.Store((((64u * min(uint(tint_symbol_save), (tint_symbol_6 - 1u))) + (16u * min(uint(tint_symbol_save_1), 3u))) + (4u * min(uint(tint_symbol_1), 3u))), asuint((asint(buffer.Load((((64u * min(uint(tint_symbol_save), (tint_symbol_6 - 1u))) + (16u * min(uint(tint_symbol_save_1), 3u))) + (4u * min(uint(tint_symbol_1), 3u))))) - 1)));
    while (true) {
      if (!((v < 10u))) {
        break;
      }
      {
      }
      {
        uint tint_symbol_7 = 0u;
        buffer.GetDimensions(tint_symbol_7);
        uint tint_symbol_8 = (tint_symbol_7 / 64u);
        int tint_symbol_2_save = idx4();
        int tint_symbol_2_save_1 = idx5();
        int tint_symbol_3 = idx6();
        buffer.Store((((64u * min(uint(tint_symbol_2_save), (tint_symbol_8 - 1u))) + (16u * min(uint(tint_symbol_2_save_1), 3u))) + (4u * min(uint(tint_symbol_3), 3u))), asuint((asint(buffer.Load((((64u * min(uint(tint_symbol_2_save), (tint_symbol_8 - 1u))) + (16u * min(uint(tint_symbol_2_save_1), 3u))) + (4u * min(uint(tint_symbol_3), 3u))))) - 1)));
      }
    }
  }
}
