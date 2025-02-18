
int get_i32() {
  return int(1);
}

void f() {
  int2 v2 = int2((get_i32()).xx);
  int3 v3 = int3((get_i32()).xxx);
  int4 v4 = int4((get_i32()).xxxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

