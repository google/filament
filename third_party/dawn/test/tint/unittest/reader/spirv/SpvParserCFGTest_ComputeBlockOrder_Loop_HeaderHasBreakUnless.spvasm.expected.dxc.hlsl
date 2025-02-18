SKIP: INVALID

static uint var_1 = 0u;

void main_1() {
  while (true) {
    if (false) {
      break;
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
hlsl.hlsl:12: error: Loop must have break.
Validation failed.



tint executable returned error: exit status 1
