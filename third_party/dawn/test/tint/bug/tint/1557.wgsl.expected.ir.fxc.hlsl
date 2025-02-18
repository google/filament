SKIP: FAILED


cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
int f() {
  return int(0);
}

void g() {
  int j = int(0);
  {
    while(true) {
      if ((j >= int(1))) {
        break;
      }
      j = (j + int(1));
      int k = f();
      {
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void main() {
  switch(asint(u[0u].x)) {
    case int(0):
    {
      switch(asint(u[0u].x)) {
        case int(0):
        {
          break;
        }
        default:
        {
          g();
          break;
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

FXC validation failure:
internal error: no storage type for block output


tint executable returned error: exit status 1
