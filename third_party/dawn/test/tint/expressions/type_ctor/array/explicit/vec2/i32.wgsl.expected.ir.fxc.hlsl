
static const int2 v_1[2] = {(int(1)).xx, (int(2)).xx};
static int2 arr[2] = v_1;
void f() {
  int2 v[2] = arr;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

