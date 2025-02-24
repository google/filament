#version 310 es
precision highp float;
precision highp int;

int foo() {
  return 1;
}
void main() {
  float arr[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  {
    uvec2 tint_loop_idx = uvec2(0u);
    uint v = min(uint(foo()), 3u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      float x = arr[v];
      break;
    }
  }
}
