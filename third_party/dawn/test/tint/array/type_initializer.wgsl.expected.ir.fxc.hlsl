
RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void main() {
  int x = int(42);
  int empty[4] = (int[4])0;
  int nonempty[4] = {int(1), int(2), int(3), int(4)};
  int nonempty_with_expr[4] = {int(1), x, (x + int(1)), nonempty[3u]};
  int nested_empty[2][3][4] = (int[2][3][4])0;
  int nested_nonempty[2][3][4] = {{{int(1), int(2), int(3), int(4)}, {int(5), int(6), int(7), int(8)}, {int(9), int(10), int(11), int(12)}}, {{int(13), int(14), int(15), int(16)}, {int(17), int(18), int(19), int(20)}, {int(21), int(22), int(23), int(24)}}};
  int v[4] = {int(1), int(2), x, (x + int(1))};
  int v_1[4] = {int(5), int(6), nonempty[2u], (nonempty[3u] + int(1))};
  int v_2[3][4] = {v, v_1, nonempty};
  int v_3[3][4] = nested_nonempty[1u];
  int nested_nonempty_with_expr[2][3][4] = {v_2, v_3};
  int subexpr_empty = int(0);
  int subexpr_nonempty = int(3);
  int v_4[4] = {int(1), x, (x + int(1)), nonempty[3u]};
  int subexpr_nonempty_with_expr = v_4[2u];
  int subexpr_nested_empty[4] = (int[4])0;
  int subexpr_nested_nonempty[4] = {int(5), int(6), int(7), int(8)};
  int v_5[4] = {int(1), x, (x + int(1)), nonempty[3u]};
  int v_6[4] = nested_nonempty[1u][2u];
  int v_7[2][4] = {v_5, v_6};
  int subexpr_nested_nonempty_with_expr[4] = v_7[1u];
  s.Store(0u, asuint((((((((((((empty[0u] + nonempty[0u]) + nonempty_with_expr[0u]) + nested_empty[0u][0u][0u]) + nested_nonempty[0u][0u][0u]) + nested_nonempty_with_expr[0u][0u][0u]) + subexpr_empty) + subexpr_nonempty) + subexpr_nonempty_with_expr) + subexpr_nested_empty[0u]) + subexpr_nested_nonempty[0u]) + subexpr_nested_nonempty_with_expr[0u])));
}

