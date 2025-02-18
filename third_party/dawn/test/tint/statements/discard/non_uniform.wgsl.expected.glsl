#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_non_uniform_global_block_ssbo {
  int inner;
} v;
layout(binding = 1, std430)
buffer f_output_block_ssbo {
  float inner;
} v_1;
bool continue_execution = true;
void main() {
  if ((v.inner < 0)) {
    continue_execution = false;
  }
  float v_2 = dFdx(1.0f);
  if (continue_execution) {
    v_1.inner = v_2;
  }
  if (!(continue_execution)) {
    discard;
  }
}
