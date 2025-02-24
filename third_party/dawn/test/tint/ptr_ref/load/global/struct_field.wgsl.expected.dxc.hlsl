struct S {
  int i;
};

static S V = (S)0;

[numthreads(1, 1, 1)]
void main() {
  int i = V.i;
  return;
}
