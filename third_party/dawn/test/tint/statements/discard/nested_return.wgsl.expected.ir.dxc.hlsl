
RWByteAddressBuffer non_uniform_global : register(u0);
void main() {
  if ((asint(non_uniform_global.Load(0u)) < int(0))) {
    discard;
  }
}

