struct S {
  int4 a[4];
};


static int counter = int(0);
int foo() {
  counter = (counter + int(1));
  return counter;
}

int bar() {
  counter = (counter + int(2));
  return counter;
}

void main() {
  S x = (S)0;
  uint v = min(uint(foo()), 3u);
  int v_1 = bar();
  int v_2 = (x.a[v][min(uint(v_1), 3u)] + int(5));
  x.a[v][min(uint(v_1), 3u)] = v_2;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

