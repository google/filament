RWByteAddressBuffer v : register(u0);

[numthreads(1, 1, 1)]
void main() {
  int u = (asint(v.Load(0u)) + 1);
  return;
}
