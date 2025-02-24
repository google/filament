struct S {
  int a;
  float b;
};


[numthreads(1, 1, 1)]
void main() {
  S v = (S)0;
}

