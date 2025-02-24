<dawn>/test/tint/bug/tint/2202.wgsl:7:9 warning: code is unreachable
        let _e9 = (vec3<i32>().y >= vec3<i32>().y);
        ^^^^^^^


[numthreads(1, 1, 1)]
void main() {
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      {
        uint2 tint_loop_idx_1 = (0u).xx;
        while(true) {
          if (all((tint_loop_idx_1 == (4294967295u).xx))) {
            break;
          }
          return;
        }
      }
      /* unreachable */
    }
  }
}

