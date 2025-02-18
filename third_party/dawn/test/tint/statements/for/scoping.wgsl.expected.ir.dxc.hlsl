
void f() {
  {
    uint2 tint_loop_idx = (0u).xx;
    int must_not_collide = int(0);
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      break;
    }
  }
  int must_not_collide = int(0);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

