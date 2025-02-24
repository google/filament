
cbuffer cbuffer_x : register(b0) {
  uint4 x[1];
};
[numthreads(1, 1, 1)]
void main() {
  switch(asint(x[0u].x)) {
    case int(0):
    {
      {
        uint2 tint_loop_idx = (0u).xx;
        while(true) {
          if (all((tint_loop_idx == (4294967295u).xx))) {
            break;
          }
          return;
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
}

