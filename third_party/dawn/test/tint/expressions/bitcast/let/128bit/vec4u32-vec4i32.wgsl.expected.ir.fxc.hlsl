
[numthreads(1, 1, 1)]
void f() {
  uint4 a = uint4(1073757184u, 3288351232u, 3296724992u, 987654321u);
  int4 b = asint(a);
}

