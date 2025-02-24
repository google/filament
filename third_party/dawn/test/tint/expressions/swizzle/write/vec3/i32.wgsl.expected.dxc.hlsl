[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  int3 v;
};

static S P = (S)0;

void f() {
  P.v = int3(1, 2, 3);
  P.v.x = 1;
  P.v.y = 2;
  P.v.z = 3;
}
