#version 310 es

uint count = 0u;
layout(binding = 1, std430)
buffer Outputs_1_ssbo {
  uint data[];
} outputs;
void push_output(uint value) {
  uint v = count;
  uint v_1 = min(v, (uint(outputs.data.length()) - 1u));
  outputs.data[v_1] = value;
  count = (count + 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
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
