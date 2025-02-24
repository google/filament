void set_vector_element(inout int4 vec, int idx, int val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  int4 a[4];
};

static int counter = 0;

int foo() {
  counter = (counter + 1);
  return counter;
}

int bar() {
  counter = (counter + 2);
  return counter;
}

void main() {
  S x = (S)0;
  int tint_symbol_save = foo();
  int tint_symbol_1 = bar();
  {
    int4 tint_symbol_3[4] = x.a;
    set_vector_element(tint_symbol_3[min(uint(tint_symbol_save), 3u)], min(uint(tint_symbol_1), 3u), (x.a[min(uint(tint_symbol_save), 3u)][min(uint(tint_symbol_1), 3u)] + 5));
    x.a = tint_symbol_3;
  }
}
