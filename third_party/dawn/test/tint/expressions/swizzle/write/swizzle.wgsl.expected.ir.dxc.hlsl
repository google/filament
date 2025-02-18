struct S {
  float3 val[3];
};


void a() {
  int4 a_1 = (int(0)).xxxx;
  a_1.x = int(1);
  a_1.z = int(2);
  S d = (S)0;
  d.val[2u].y = 3.0f;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

