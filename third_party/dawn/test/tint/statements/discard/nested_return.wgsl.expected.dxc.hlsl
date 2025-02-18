RWByteAddressBuffer non_uniform_global : register(u0);

void main() {
  if ((asint(non_uniform_global.Load(0u)) < 0)) {
    discard;
  }
  {
    {
      return;
    }
  }
  return;
}
