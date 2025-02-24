
int foo() {
  return int(1);
}

void main() {
  float arr[4] = (float[4])0;
  {
    uint2 tint_loop_idx = (0u).xx;
    uint v = min(uint(foo()), 3u);
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      float x = arr[v];
      break;
    }
  }
}

