
void f_1() {
  int3 v = (int(0)).xxx;
  int3 n = (int(0)).xxx;
  uint offset_1 = 0u;
  uint count = 0u;
  int3 v_1 = v;
  int3 v_2 = n;
  uint v_3 = offset_1;
  uint v_4 = (v_3 + count);
  uint v_5 = (((v_3 < 32u)) ? ((1u << v_3)) : (0u));
  uint v_6 = ((v_5 - 1u) ^ ((((v_4 < 32u)) ? ((1u << v_4)) : (0u)) - 1u));
  int3 v_7 = (((v_3 < 32u)) ? ((v_2 << uint3((v_3).xxx))) : ((int(0)).xxx));
  int3 v_8 = (v_7 & int3((v_6).xxx));
  int3 x_16 = (v_8 | (v_1 & int3((~(v_6)).xxx)));
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
}

