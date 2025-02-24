
static const int v_1[2][2] = {{int(1), int(2)}, {int(3), int(4)}};
static int arr[2][2] = v_1;
void f() {
  int v[2][2] = arr;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

