
void f_1() {
  uint v = 0u;
  uint offset_1 = 0u;
  uint count = 0u;
  uint v_1 = v;
  uint v_2 = min(offset_1, 32u);
  uint v_3 = (32u - min(32u, (v_2 + count)));
  uint v_4 = (((v_3 < 32u)) ? ((v_1 << uint(v_3))) : (0u));
  uint x_11 = ((((v_3 + v_2) < 32u)) ? ((v_4 >> uint((v_3 + v_2)))) : (((v_4 >> 31u) >> 1u)));
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
}

