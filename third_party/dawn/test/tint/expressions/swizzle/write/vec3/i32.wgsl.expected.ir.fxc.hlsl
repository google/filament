struct S {
  int3 v;
};


static S P = (S)0;
void f() {
  P.v = int3(int(1), int(2), int(3));
  P.v.x = int(1);
  P.v.y = int(2);
  P.v.z = int(3);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

