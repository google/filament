RWByteAddressBuffer s : register(u0);

[numthreads(1, 1, 1)]
void main() {
  int x = 42;
  int empty[4] = (int[4])0;
  int nonempty[4] = {1, 2, 3, 4};
  int nonempty_with_expr[4] = {1, x, (x + 1), nonempty[3]};
  int nested_empty[2][3][4] = (int[2][3][4])0;
  int nested_nonempty[2][3][4] = {{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}}, {{13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24}}};
  int tint_symbol[4] = {1, 2, x, (x + 1)};
  int tint_symbol_1[4] = {5, 6, nonempty[2], (nonempty[3] + 1)};
  int tint_symbol_2[3][4] = {tint_symbol, tint_symbol_1, nonempty};
  int nested_nonempty_with_expr[2][3][4] = {tint_symbol_2, nested_nonempty[1]};
  int subexpr_empty = 0;
  int subexpr_nonempty = 3;
  int tint_symbol_3[4] = {1, x, (x + 1), nonempty[3]};
  int subexpr_nonempty_with_expr = tint_symbol_3[2];
  int subexpr_nested_empty[4] = (int[4])0;
  int subexpr_nested_nonempty[4] = {5, 6, 7, 8};
  int tint_symbol_4[4] = {1, x, (x + 1), nonempty[3]};
  int tint_symbol_5[2][4] = {tint_symbol_4, nested_nonempty[1][2]};
  int subexpr_nested_nonempty_with_expr[4] = tint_symbol_5[1];
  s.Store(0u, asuint((((((((((((empty[0] + nonempty[0]) + nonempty_with_expr[0]) + nested_empty[0][0][0]) + nested_nonempty[0][0][0]) + nested_nonempty_with_expr[0][0][0]) + subexpr_empty) + subexpr_nonempty) + subexpr_nonempty_with_expr) + subexpr_nested_empty[0]) + subexpr_nested_nonempty[0]) + subexpr_nested_nonempty_with_expr[0])));
  return;
}
