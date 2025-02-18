
void f_1() {
  uint v = 0u;
  uint n = 0u;
  uint offset_1 = 0u;
  uint count = 0u;
  uint v_1 = v;
  uint v_2 = n;
  uint v_3 = offset_1;
  uint v_4 = (v_3 + count);
  uint v_5 = (((v_3 < 32u)) ? ((1u << v_3)) : (0u));
  uint v_6 = ((v_5 - 1u) ^ ((((v_4 < 32u)) ? ((1u << v_4)) : (0u)) - 1u));
  uint v_7 = (((v_3 < 32u)) ? ((v_2 << uint(v_3))) : (0u));
  uint v_8 = (v_7 & uint(v_6));
  uint x_12 = (v_8 | (v_1 & uint(~(v_6))));
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
}

