SKIP: INVALID

static uint var_1 = 0u;

void main_1() {
  var_1 = 0u;
  while (true) {
    var_1 = 1u;
    {
      if (false) { break; }
    }
  }
  var_1 = 5u;
  return;
}

void main() {
  main_1();
  return;
}
DXC validation failure:
error: validation errors
hlsl.hlsl:15: error: Loop must have break.
Validation failed.



tint executable returned error: exit status 1
