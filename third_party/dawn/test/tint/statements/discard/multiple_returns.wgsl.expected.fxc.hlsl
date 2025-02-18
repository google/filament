static bool tint_discarded = false;
RWByteAddressBuffer non_uniform_global : register(u0);
RWByteAddressBuffer output : register(u1);

void main() {
  if ((asint(non_uniform_global.Load(0u)) < 0)) {
    tint_discarded = true;
  }
  float tint_symbol = ddx(1.0f);
  if (!(tint_discarded)) {
    output.Store(0u, asuint(tint_symbol));
  }
  if ((asfloat(output.Load(0u)) < 0.0f)) {
    int i = 0;
    while (true) {
      if ((asfloat(output.Load(0u)) > float(i))) {
        if (!(tint_discarded)) {
          output.Store(0u, asuint(float(i)));
        }
        if (tint_discarded) {
          discard;
        }
        return;
      }
      {
        i = (i + 1);
        if ((i == 5)) { break; }
      }
    }
    if (tint_discarded) {
      discard;
    }
    return;
  }
  if (tint_discarded) {
    discard;
  }
  return;
}
