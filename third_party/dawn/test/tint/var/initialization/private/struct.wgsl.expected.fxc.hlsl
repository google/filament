struct S {
  int a;
  float b;
};

static S v = (S)0;

[numthreads(1, 1, 1)]
void main() {
  return;
}
