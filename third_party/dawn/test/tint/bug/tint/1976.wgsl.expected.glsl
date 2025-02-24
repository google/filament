#version 310 es


struct Results {
  float colorSamples[4];
};

layout(binding = 2, std430)
buffer results_block_1_ssbo {
  Results inner;
} v;
uniform highp sampler2DMS texture0;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec2 v_1 = (uvec2(textureSize(texture0)) - uvec2(1u));
  v.inner.colorSamples[0u] = texelFetch(texture0, ivec2(min(uvec2(ivec2(0)), v_1)), 0).x;
}
