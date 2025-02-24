SKIP: INVALID

static uint x_1 = 0u;
static bool x_7 = false;
static bool x_8 = false;

void main_1() {
  bool x_101 = x_7;
  bool x_102 = x_8;
  while (true) {
    uint x_2 = 0u;
    if (x_101) {
      break;
    }
    if (x_102) {
      x_2 = 0u;
      {
        x_1 = x_2;
      }
      continue;
    } else {
      x_2 = 1u;
      {
        x_1 = x_2;
      }
      continue;
    }
    {
      x_1 = x_2;
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
hlsl.hlsl:33: error: Loop must have break.
Validation failed.



tint executable returned error: exit status 1
