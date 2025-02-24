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
    x_2 = 0u;
    if (x_102) {
      x_2 = 1u;
      {
        x_1 = x_2;
      }
      continue;
    } else {
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
FXC validation failure:
<scrubbed_path>(8,10-13): error X3696: infinite loop detected - loop never exits


tint executable returned error: exit status 1
