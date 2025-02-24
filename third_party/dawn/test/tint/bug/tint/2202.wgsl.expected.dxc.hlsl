<dawn>/test/tint/bug/tint/2202.wgsl:7:9 warning: code is unreachable
        let _e9 = (vec3<i32>().y >= vec3<i32>().y);
        ^^^^^^^

[numthreads(1, 1, 1)]
void main() {
  while (true) {
    while (true) {
      return;
    }
    bool _e9 = true;
    {
      bool tint_tmp = _e9;
      if (!tint_tmp) {
        tint_tmp = _e9;
      }
      if ((tint_tmp)) { break; }
    }
  }
  return;
}
