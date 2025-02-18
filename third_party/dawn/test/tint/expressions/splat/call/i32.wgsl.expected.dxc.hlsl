[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int get_i32() {
  return 1;
}

void f() {
  int tint_symbol = get_i32();
  int2 v2 = int2((tint_symbol).xx);
  int tint_symbol_1 = get_i32();
  int3 v3 = int3((tint_symbol_1).xxx);
  int tint_symbol_2 = get_i32();
  int4 v4 = int4((tint_symbol_2).xxxx);
}
