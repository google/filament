SKIP: INVALID

static uint x_1 = 0u;
static bool x_7 = false;
static bool x_8 = false;

void main_1() {
  while (true) {
    uint x_2 = 0u;
    uint x_3 = 0u;
    bool x_101 = x_7;
    bool x_102 = x_8;
    x_2 = 0u;
    x_3 = 1u;
    if (x_101) {
      break;
    }
    while (true) {
      uint x_4 = 0u;
      if (x_102) {
        break;
      }
      {
        x_4 = (x_2 + 1u);
        uint x_3_c30 = x_3;
        x_2 = x_4;
        x_3 = x_3_c30;
      }
    }
  }
  return;
}

void main() {
  main_1();
  return;
}
DXC validation failure:
error: validation errors
hlsl.hlsl:32: error: Loop must have break.
Validation failed.



tint executable returned error: exit status 1
