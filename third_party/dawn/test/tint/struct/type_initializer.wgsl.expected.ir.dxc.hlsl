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
  int x = int(42);
  S1 empty = (S1)0;
  S1 nonempty = {int(1), int(2), int(3), int(4)};
  S1 nonempty_with_expr = {int(1), x, (x + int(1)), nonempty.d};
  S3 nested_empty = (S3)0;
  S3 nested_nonempty = {int(1), {int(2), int(3), int(4), int(5)}, {int(6), {int(7), int(8), int(9), int(10)}}};
  S1 v = {int(2), x, (x + int(1)), nested_nonempty.i.f.d};
  S2 v_1 = {int(6), nonempty};
  S3 nested_nonempty_with_expr = {int(1), v, v_1};
  int subexpr_empty = int(0);
  int subexpr_nonempty = int(2);
  S1 v_2 = {int(1), x, (x + int(1)), nonempty.d};
  int subexpr_nonempty_with_expr = v_2.c;
  S1 subexpr_nested_empty = (S1)0;
  S1 subexpr_nested_nonempty = {int(2), int(3), int(4), int(5)};
  S1 v_3 = {int(2), x, (x + int(1)), nested_nonempty.i.f.d};
  S2 v_4 = {int(1), v_3};
  S1 subexpr_nested_nonempty_with_expr = v_4.f;
  T aosoa_empty[2] = (T[2])0;
  T aosoa_nonempty[2] = {{{int(1), int(2)}}, {{int(3), int(4)}}};
  int v_5[2] = {int(1), (aosoa_nonempty[0u].a[0u] + int(1))};
  T v_6 = {v_5};
  T v_7 = aosoa_nonempty[1u];
  T aosoa_nonempty_with_expr[2] = {v_6, v_7};
}

