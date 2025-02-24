#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_weights_block_ubo {
  vec2 inner;
} v;
void main() {
  float a = v.inner.x;
}
