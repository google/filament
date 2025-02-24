
RWByteAddressBuffer buf : register(u0);
[numthreads(1, 1, 1)]
void main() {
  buf.Store(0u, asuint(int(12)));
}

