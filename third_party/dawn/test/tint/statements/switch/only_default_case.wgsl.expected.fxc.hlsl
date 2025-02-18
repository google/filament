[numthreads(1, 1, 1)]
void f() {
  int i = 0;
  int result = 0;
  do {
    result = 44;
    break;
  } while (false);
  return;
}
