[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  float3 val[3];
};

void a() {
  int4 a_1 = (0).xxxx;
  a_1.x = 1;
  a_1.z = 2;
  S d = (S)0;
  d.val[2].y = 3.0f;
}
