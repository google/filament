#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 3, std430)
buffer Result_1_ssbo {
  float values[];
} result;
layout(binding = 0, std140)
uniform tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v;
uniform highp sampler2DArray myTexture;
void main_inner(uvec3 GlobalInvocationID) {
  uint flatIndex = (((4u * GlobalInvocationID.z) + (2u * GlobalInvocationID.y)) + GlobalInvocationID.x);
  flatIndex = (flatIndex * 1u);
  ivec2 v_1 = ivec2(GlobalInvocationID.xy);
  uint v_2 = (uint(textureSize(myTexture, 0).z) - 1u);
  uint v_3 = min(uint(0), v_2);
  uint v_4 = (v.inner.tint_builtin_value_0 - 1u);
  uint v_5 = min(uint(0), v_4);
  uvec2 v_6 = (uvec2(textureSize(myTexture, int(v_5)).xy) - uvec2(1u));
  ivec2 v_7 = ivec2(min(uvec2(v_1), v_6));
  ivec3 v_8 = ivec3(v_7, int(v_3));
  vec4 texel = texelFetch(myTexture, v_8, int(v_5));
  {
    uint i = 0u;
    while(true) {
      if ((i < 1u)) {
      } else {
        break;
      }
      uint v_9 = (flatIndex + i);
      uint v_10 = min(v_9, (uint(result.values.length()) - 1u));
      result.values[v_10] = texel.x;
      {
        i = (i + 1u);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_GlobalInvocationID);
}
