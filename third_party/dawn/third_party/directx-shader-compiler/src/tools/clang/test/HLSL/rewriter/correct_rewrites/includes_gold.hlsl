// Rewrite unchanged result:
int includedFunc(int a) {
  return a + 50;
}


int includedFunc2(int c) {
  return c + 100;
}


int includedFunc3(int e) {
  return e - 200;
}


int func1(int b) {
  return includedFunc(b);
}


int func2(int d) {
  return includedFunc2(d);
}


int func3(int f) {
  return includedFunc3(f);
}


