[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  float3 val[3];
};

void a() {
  int4 a_1 = (0).xxxx;
  int b = a_1.x;
  int4 c = a_1.zzyy;
  S d = (S)0;
  float3 e = d.val[2].yzx;
}
