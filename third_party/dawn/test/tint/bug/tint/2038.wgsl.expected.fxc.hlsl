RWByteAddressBuffer output : register(u0);

[numthreads(1, 1, 1)]
void main() {
  if (false) {
    output.Store(0u, asuint(1u));
  }
  if (false) {
    output.Store(4u, asuint(1u));
  }
  return;
}
