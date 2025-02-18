
void f_1() {
  int v = int(0);
  int n = int(0);
  uint offset_1 = 0u;
  uint count = 0u;
  int v_1 = v;
  int v_2 = n;
  uint v_3 = offset_1;
  uint v_4 = (v_3 + count);
  uint v_5 = (((v_3 < 32u)) ? ((1u << v_3)) : (0u));
  uint v_6 = ((v_5 - 1u) ^ ((((v_4 < 32u)) ? ((1u << v_4)) : (0u)) - 1u));
  int v_7 = (((v_3 < 32u)) ? ((v_2 << uint(v_3))) : (int(0)));
  int v_8 = (v_7 & int(v_6));
  int x_15 = (v_8 | (v_1 & int(~(v_6))));
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
}

