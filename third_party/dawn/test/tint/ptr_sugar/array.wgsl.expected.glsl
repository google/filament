#version 310 es

void deref_const() {
  int a[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int b = a[0u];
  a[0u] = 42;
}
void no_deref_const() {
  int a[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int b = a[0u];
  a[0u] = 42;
}
void deref_let() {
  int a[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int i = 0;
  uint v = min(uint(i), 9u);
  int b = a[v];
  a[0u] = 42;
}
void no_deref_let() {
  int a[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int i = 0;
  uint v_1 = min(uint(i), 9u);
  int b = a[v_1];
  a[0u] = 42;
}
void deref_var() {
  int a[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int i = 0;
  uint v_2 = min(uint(i), 9u);
  int b = a[v_2];
  a[0u] = 42;
}
void no_deref_var() {
  int a[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int i = 0;
  uint v_3 = min(uint(i), 9u);
  int b = a[v_3];
  a[0u] = 42;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  deref_const();
  no_deref_const();
  deref_let();
  no_deref_let();
  deref_var();
  no_deref_var();
}
