RWByteAddressBuffer buf : register(u0);

[numthreads(1, 1, 1)]
void main() {
  buf.Store(0u, asuint(12));
  return;
}
