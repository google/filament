
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
[numthreads(1, 1, 1)]
void main() {
  v_1.Store4(0u, asuint(asfloat(v.Load4(0u))));
}

