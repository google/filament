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
  if (tint_discarded) {
    discard;
  }
  return;
}
