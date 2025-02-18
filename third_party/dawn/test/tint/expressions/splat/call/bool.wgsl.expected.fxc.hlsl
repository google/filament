[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

bool get_bool() {
  return true;
}

void f() {
  bool tint_symbol = get_bool();
  bool2 v2 = bool2((tint_symbol).xx);
  bool tint_symbol_1 = get_bool();
  bool3 v3 = bool3((tint_symbol_1).xxx);
  bool tint_symbol_2 = get_bool();
  bool4 v4 = bool4((tint_symbol_2).xxxx);
}
