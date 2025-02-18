[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  uint3 v;
};

static S P = (S)0;

void f() {
  P.v = uint3(1u, 2u, 3u);
  P.v.x = 1u;
  P.v.y = 2u;
  P.v.z = 3u;
}
