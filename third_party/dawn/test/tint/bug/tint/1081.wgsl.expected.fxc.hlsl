static bool tint_discarded = false;

int f(int x) {
  if ((x == 10)) {
    tint_discarded = true;
  }
  return x;
}

struct tint_symbol_1 {
  nointerpolation int3 x : TEXCOORD1;
};
struct tint_symbol_2 {
  int value : SV_Target2;
};

int main_inner(int3 x) {
  int y = x.x;
  while (true) {
    int r = f(y);
    if ((r == 0)) {
      break;
    }
  }
  return y;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  int inner_result = main_inner(tint_symbol.x);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  if (tint_discarded) {
    discard;
  }
  return wrapper_result;
}
