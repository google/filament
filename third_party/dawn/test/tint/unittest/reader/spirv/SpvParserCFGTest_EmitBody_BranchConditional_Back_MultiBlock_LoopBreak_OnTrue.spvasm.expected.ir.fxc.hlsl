SKIP: INVALID


static uint var_1 = 0u;
void main_1() {
  var_1 = 0u;
  {
    while(true) {
      var_1 = 1u;
      {
        if (false) { break; }
      }
      continue;
    }
  }
  var_1 = 5u;
}

void main() {
  main_1();
}

FXC validation failure:
<scrubbed_path>(6,11-14): error X3696: infinite loop detected - loop never exits


tint executable returned error: exit status 1
