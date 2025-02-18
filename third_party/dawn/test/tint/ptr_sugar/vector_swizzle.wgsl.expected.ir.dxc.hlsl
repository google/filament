
RWByteAddressBuffer buffer : register(u0);
void deref() {
  buffer.Store4(0u, asuint(asint(buffer.Load4(0u)).wzyx));
}

void no_deref() {
  buffer.Store4(0u, asuint(asint(buffer.Load4(0u)).wzyx));
}

[numthreads(1, 1, 1)]
void main() {
  deref();
  no_deref();
}

