#version 310 es


struct Uniforms {
  uint dstTextureFlipY;
  uint channelCount;
  uvec2 srcCopyOrigin;
  uvec2 dstCopyOrigin;
  uvec2 copySize;
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
bool aboutEqual(float value, float expect) {
  return (abs((value - expect)) < 0.00100000004749745131f);
}
void main_inner(uvec3 GlobalInvocationID) {
  uvec2 srcSize = uvec2(textureSize(src, 0));
  uvec2 dstSize = uvec2(textureSize(dst, 0));
  uvec2 dstTexCoord = uvec2(GlobalInvocationID.xy);
  vec4 nonCoveredColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
  bool success = true;
  bool v_3 = false;
  if ((dstTexCoord.x < v_1.inner.dstCopyOrigin.x)) {
    v_3 = true;
  } else {
    v_3 = (dstTexCoord.y < v_1.inner.dstCopyOrigin.y);
  }
  bool v_4 = false;
  if (v_3) {
    v_4 = true;
  } else {
    v_4 = (dstTexCoord.x >= (v_1.inner.dstCopyOrigin.x + v_1.inner.copySize.x));
  }
  bool v_5 = false;
  if (v_4) {
    v_5 = true;
  } else {
    v_5 = (dstTexCoord.y >= (v_1.inner.dstCopyOrigin.y + v_1.inner.copySize.y));
  }
  if (v_5) {
    bool v_6 = false;
    if (success) {
      ivec2 v_7 = ivec2(dstTexCoord);
      uint v_8 = (v_2.inner.tint_builtin_value_1 - 1u);
      uint v_9 = min(uint(0), v_8);
      uvec2 v_10 = (uvec2(textureSize(dst, int(v_9))) - uvec2(1u));
      ivec2 v_11 = ivec2(min(uvec2(v_7), v_10));
      v_6 = all(equal(texelFetch(dst, v_11, int(v_9)), nonCoveredColor));
    } else {
      v_6 = false;
    }
    success = v_6;
  } else {
    uvec2 srcTexCoord = ((dstTexCoord - v_1.inner.dstCopyOrigin) + v_1.inner.srcCopyOrigin);
    if ((v_1.inner.dstTextureFlipY == 1u)) {
      srcTexCoord.y = ((srcSize.y - srcTexCoord.y) - 1u);
    }
    ivec2 v_12 = ivec2(srcTexCoord);
    uint v_13 = (v_2.inner.tint_builtin_value_0 - 1u);
    uint v_14 = min(uint(0), v_13);
    uvec2 v_15 = (uvec2(textureSize(src, int(v_14))) - uvec2(1u));
    ivec2 v_16 = ivec2(min(uvec2(v_12), v_15));
    vec4 srcColor = texelFetch(src, v_16, int(v_14));
    ivec2 v_17 = ivec2(dstTexCoord);
    uint v_18 = (v_2.inner.tint_builtin_value_1 - 1u);
    uint v_19 = min(uint(0), v_18);
    uvec2 v_20 = (uvec2(textureSize(dst, int(v_19))) - uvec2(1u));
    ivec2 v_21 = ivec2(min(uvec2(v_17), v_20));
    vec4 dstColor = texelFetch(dst, v_21, int(v_19));
    if ((v_1.inner.channelCount == 2u)) {
      bool v_22 = false;
      if (success) {
        v_22 = aboutEqual(dstColor.x, srcColor.x);
      } else {
        v_22 = false;
      }
      bool v_23 = false;
      if (v_22) {
        v_23 = aboutEqual(dstColor.y, srcColor.y);
      } else {
        v_23 = false;
      }
      success = v_23;
    } else {
      bool v_24 = false;
      if (success) {
        v_24 = aboutEqual(dstColor.x, srcColor.x);
      } else {
        v_24 = false;
      }
      bool v_25 = false;
      if (v_24) {
        v_25 = aboutEqual(dstColor.y, srcColor.y);
      } else {
        v_25 = false;
      }
      bool v_26 = false;
      if (v_25) {
        v_26 = aboutEqual(dstColor.z, srcColor.z);
      } else {
        v_26 = false;
      }
      bool v_27 = false;
      if (v_26) {
        v_27 = aboutEqual(dstColor.w, srcColor.w);
      } else {
        v_27 = false;
      }
      success = v_27;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * dstSize.x) + GlobalInvocationID.x);
  if (success) {
    uint v_28 = min(outputIndex, (uint(v.result.length()) - 1u));
    v.result[v_28] = 1u;
  } else {
    uint v_29 = min(outputIndex, (uint(v.result.length()) - 1u));
    v.result[v_29] = 0u;
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_GlobalInvocationID);
}
