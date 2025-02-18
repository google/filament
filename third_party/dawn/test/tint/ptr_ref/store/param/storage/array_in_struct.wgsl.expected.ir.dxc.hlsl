
RWByteAddressBuffer S : register(u0);
void v(uint offset, int obj[4]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      S.Store((offset + (v_2 * 4u)), asuint(obj[v_2]));
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
}

void func() {
  int v_3[4] = (int[4])0;
  v(0u, v_3);
}

[numthreads(1, 1, 1)]
void main() {
  func();
}

