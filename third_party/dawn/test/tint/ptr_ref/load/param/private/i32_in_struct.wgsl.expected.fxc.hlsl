struct str {
  int i;
};

int func(inout int pointer) {
  return pointer;
}

static str P = (str)0;

[numthreads(1, 1, 1)]
void main() {
  int r = func(P.i);
  return;
}
