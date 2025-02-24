int foo() {
  return 1;
}

void main() {
  float arr[4] = (float[4])0;
  int a_save = foo();
  {
    for(; ; ) {
      float x = arr[min(uint(a_save), 3u)];
      break;
    }
  }
  return;
}
