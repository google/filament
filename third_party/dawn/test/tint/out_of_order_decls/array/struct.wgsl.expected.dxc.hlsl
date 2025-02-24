struct S {
  int m;
};

static S A[4] = (S[4])0;

void f() {
  S tint_symbol = {1};
  A[0] = tint_symbol;
  return;
}
