SKIP: INVALID


cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);
[numthreads(1, 1, 1)]
void main() {
  float16_t x = float16_t(f16tof32(u[0u].x));
  s.Store<float16_t>(0u, x);
}

FXC validation failure:
<scrubbed_path>(8,3-11): error X3000: unrecognized identifier 'float16_t'
<scrubbed_path>(8,13): error X3000: unrecognized identifier 'x'


tint executable returned error: exit status 1
