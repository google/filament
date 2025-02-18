struct S2 {
  int m;
};
struct S1 {
  S2 m;
};

void f() {
  S1 v = (S1)0;
  return;
}
