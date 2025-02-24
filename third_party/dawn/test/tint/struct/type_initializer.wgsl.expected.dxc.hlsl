struct S1 {
  int a;
  int b;
  int c;
  int d;
};
struct S2 {
  int e;
  S1 f;
};
struct S3 {
  int g;
  S1 h;
  S2 i;
};
struct T {
  int a[2];
};

[numthreads(1, 1, 1)]
void main() {
  int x = 42;
  S1 empty = (S1)0;
  S1 nonempty = {1, 2, 3, 4};
  S1 nonempty_with_expr = {1, x, (x + 1), nonempty.d};
  S3 nested_empty = (S3)0;
  S3 nested_nonempty = {1, {2, 3, 4, 5}, {6, {7, 8, 9, 10}}};
  S1 tint_symbol = {2, x, (x + 1), nested_nonempty.i.f.d};
  S2 tint_symbol_1 = {6, nonempty};
  S3 nested_nonempty_with_expr = {1, tint_symbol, tint_symbol_1};
  int subexpr_empty = 0;
  int subexpr_nonempty = 2;
  S1 tint_symbol_2 = {1, x, (x + 1), nonempty.d};
  int subexpr_nonempty_with_expr = tint_symbol_2.c;
  S1 subexpr_nested_empty = (S1)0;
  S1 subexpr_nested_nonempty = {2, 3, 4, 5};
  S1 tint_symbol_3 = {2, x, (x + 1), nested_nonempty.i.f.d};
  S2 tint_symbol_4 = {1, tint_symbol_3};
  S1 subexpr_nested_nonempty_with_expr = tint_symbol_4.f;
  T aosoa_empty[2] = (T[2])0;
  T aosoa_nonempty[2] = {{{1, 2}}, {{3, 4}}};
  int tint_symbol_5[2] = {1, (aosoa_nonempty[0].a[0] + 1)};
  T tint_symbol_6 = {tint_symbol_5};
  T aosoa_nonempty_with_expr[2] = {tint_symbol_6, aosoa_nonempty[1]};
  return;
}
