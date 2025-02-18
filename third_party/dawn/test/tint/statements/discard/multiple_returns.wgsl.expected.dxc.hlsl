RWByteAddressBuffer non_uniform_global : register(u0);
RWByteAddressBuffer output : register(u1);

void main() {
  if ((asint(non_uniform_global.Load(0u)) < 0)) {
    discard;
  }
  output.Store(0u, asuint(ddx(1.0f)));
  if ((asfloat(output.Load(0u)) < 0.0f)) {
    int i = 0;
    while (true) {
      if ((asfloat(output.Load(0u)) > float(i))) {
        output.Store(0u, asuint(float(i)));
        return;
      }
      {
        i = (i + 1);
        if ((i == 5)) { break; }
      }
    }
    return;
  }
  return;
}
