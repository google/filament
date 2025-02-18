[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWTexture2D<int4> tex : register(u2);

void foo() {
  {
    int i = 0;
    while (true) {
      if (!((i < 3))) {
        break;
      }
      {
      }
      {
        tex[(0).xx] = (0).xxxx;
      }
    }
  }
}
