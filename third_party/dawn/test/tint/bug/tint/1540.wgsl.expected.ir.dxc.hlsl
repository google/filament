struct S {
  bool e;
};


[numthreads(1, 1, 1)]
void main() {
  bool b = false;
  S v = {(true & b)};
}

