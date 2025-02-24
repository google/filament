[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  float3 v;
};

static S P = (S)0;

void f() {
  P.v = float3(1.0f, 2.0f, 3.0f);
  P.v.x = 1.0f;
  P.v.y = 2.0f;
  P.v.z = 3.0f;
}
