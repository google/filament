cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};

int f() {
  return 0;
}

void g() {
  int j = 0;
  while (true) {
    if ((j >= 1)) {
      break;
    }
    j = (j + 1);
    int k = f();
  }
}

[numthreads(1, 1, 1)]
void main() {
  switch(asint(u[0].x)) {
    case 0: {
      switch(asint(u[0].x)) {
        case 0: {
          break;
        }
        default: {
          g();
          break;
        }
      }
      break;
    }
    default: {
      break;
    }
  }
  return;
}
