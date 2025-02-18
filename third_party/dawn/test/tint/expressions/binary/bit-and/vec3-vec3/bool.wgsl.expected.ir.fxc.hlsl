
[numthreads(1, 1, 1)]
void f() {
  bool3 a = bool3(true, true, false);
  bool3 b = bool3(true, false, true);
  bool3 r = (a & b);
}

