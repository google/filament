
RWByteAddressBuffer non_uniform_global : register(u0);
RWByteAddressBuffer output : register(u1);
void main() {
  if ((asint(non_uniform_global.Load(0u)) < int(0))) {
    discard;
  }
  output.Store(0u, asuint(ddx(1.0f)));
}

