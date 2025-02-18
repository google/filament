static bool tint_discarded = false;

int tint_ftoi(float v) {
  return ((v <= 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

Texture2D<float4> t : register(t0);
SamplerState s : register(s1);
RWByteAddressBuffer a : register(u2);

struct tint_symbol_3 {
  float tint_symbol : TEXCOORD0;
  float2 coord : TEXCOORD1;
};
struct tint_symbol_4 {
  int value : SV_Target0;
};

int aatomicAdd(uint offset, int value) {
  int original_value = 0;
  a.InterlockedAdd(offset, value, original_value);
  return original_value;
}


int foo_inner(float tint_symbol, float2 coord) {
  if ((tint_symbol == 0.0f)) {
    tint_discarded = true;
  }
  float4 tint_symbol_1 = t.Sample(s, coord);
  int result = tint_ftoi(tint_symbol_1.x);
  {
    int i = 0;
    while (true) {
      if (!((i < 10))) {
        break;
      }
      {
        result = (result + i);
      }
      {
        int tint_symbol_5 = 0;
        if (!(tint_discarded)) {
          tint_symbol_5 = aatomicAdd(0u, 1);
        }
        i = tint_symbol_5;
      }
    }
  }
  return result;
}

tint_symbol_4 foo(tint_symbol_3 tint_symbol_2) {
  int inner_result = foo_inner(tint_symbol_2.tint_symbol, tint_symbol_2.coord);
  tint_symbol_4 wrapper_result = (tint_symbol_4)0;
  wrapper_result.value = inner_result;
  if (tint_discarded) {
    discard;
  }
  return wrapper_result;
}
