#version 310 es


struct Uniforms {
  uint dstTextureFlipY;
  uint isFloat16;
  uint isRGB10A2Unorm;
  uint channelCount;
};

struct TintTextureUniformData {
  uint tint_builtin_value_0;
  uint tint_builtin_value_1;
};

layout(binding = 2, std430)
buffer OutputBuf_1_ssbo {
  uint result[];
} v;
layout(binding = 3, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v_1;
layout(binding = 0, std140)
uniform tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v_2;
uniform highp sampler2D src;
uniform highp sampler2D dst;
uint ConvertToFp16FloatValue(float fp32) {
  return 1u;
}
uvec4 tint_v4f32_to_v4u32(vec4 value) {
  return mix(uvec4(4294967295u), mix(uvec4(0u), uvec4(value), greaterThanEqual(value, vec4(0.0f))), lessThanEqual(value, vec4(4294967040.0f)));
}
void main_inner(uvec3 GlobalInvocationID) {
  uvec2 size = uvec2(textureSize(src, 0));
  uvec2 dstTexCoord = GlobalInvocationID.xy;
  uvec2 srcTexCoord = dstTexCoord;
  if ((v_1.inner.dstTextureFlipY == 1u)) {
    srcTexCoord.y = ((size.y - dstTexCoord.y) - 1u);
  }
  uvec2 v_3 = srcTexCoord;
  uint v_4 = (v_2.inner.tint_builtin_value_0 - 1u);
  uint v_5 = min(uint(0), v_4);
  ivec2 v_6 = ivec2(min(v_3, (uvec2(textureSize(src, int(v_5))) - uvec2(1u))));
  vec4 srcColor = texelFetch(src, v_6, int(v_5));
  uvec2 v_7 = dstTexCoord;
  uint v_8 = (v_2.inner.tint_builtin_value_1 - 1u);
  uint v_9 = min(uint(0), v_8);
  ivec2 v_10 = ivec2(min(v_7, (uvec2(textureSize(dst, int(v_9))) - uvec2(1u))));
  vec4 dstColor = texelFetch(dst, v_10, int(v_9));
  bool success = true;
  uvec4 srcColorBits = uvec4(0u);
  uvec4 dstColorBits = tint_v4f32_to_v4u32(dstColor);
  {
    uvec2 tint_loop_idx = uvec2(0u);
    uint i = 0u;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if ((i < v_1.inner.channelCount)) {
      } else {
        break;
      }
      uint v_11 = i;
      srcColorBits[min(v_11, 3u)] = ConvertToFp16FloatValue(srcColor[min(i, 3u)]);
      bool v_12 = false;
      if (success) {
        v_12 = (srcColorBits[min(i, 3u)] == dstColorBits[min(i, 3u)]);
      } else {
        v_12 = false;
      }
      success = v_12;
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        i = (i + 1u);
      }
      continue;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * uint(size.x)) + GlobalInvocationID.x);
  if (success) {
    uint v_13 = outputIndex;
    uint v_14 = min(v_13, (uint(v.result.length()) - 1u));
    v.result[v_14] = 1u;
  } else {
    uint v_15 = outputIndex;
    uint v_16 = min(v_15, (uint(v.result.length()) - 1u));
    v.result[v_16] = 0u;
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_GlobalInvocationID);
}
