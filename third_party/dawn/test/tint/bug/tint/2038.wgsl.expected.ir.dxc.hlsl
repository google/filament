
RWByteAddressBuffer output : register(u0);
[numthreads(1, 1, 1)]
void main() {
  if (false) {
    output.Store(0u, 1u);
  }
  if (false) {
    output.Store(4u, 1u);
  }
}

