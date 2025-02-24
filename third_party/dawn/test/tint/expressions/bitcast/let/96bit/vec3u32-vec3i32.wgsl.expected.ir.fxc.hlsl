
[numthreads(1, 1, 1)]
void f() {
  uint3 a = uint3(1073757184u, 3288351232u, 3296724992u);
  int3 b = asint(a);
}

