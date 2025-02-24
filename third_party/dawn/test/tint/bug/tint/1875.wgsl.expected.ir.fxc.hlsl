
static uint count = 0u;
RWByteAddressBuffer outputs : register(u1);
void push_output(uint value) {
  uint v = 0u;
  outputs.GetDimensions(v);
  outputs.Store((0u + (min(count, ((v / 4u) - 1u)) * 4u)), value);
  count = (count + 1u);
}

[numthreads(1, 1, 1)]
void main() {
  uint a = 0u;
  uint b = 10u;
  uint c = 4294967294u;
  a = (a + 1u);
  b = (b + 1u);
  c = (c + 1u);
  push_output(a);
  push_output(b);
  push_output(c);
}

