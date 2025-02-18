#version 310 es


struct Params {
  uint filterDim;
  uint blockDim;
};

struct Flip {
  uint value;
};

struct TintTextureUniformData {
  uint tint_builtin_value_0;
  uint tint_builtin_value_1;
};

layout(binding = 1, std140)
uniform params_block_1_ubo {
  Params inner;
} v;
layout(binding = 2, rgba8) uniform highp writeonly image2D outputTex;
layout(binding = 3, std140)
uniform flip_block_1_ubo {
  Flip inner;
} v_1;
shared vec3 tile[4][256];
layout(binding = 0, std140)
uniform tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v_2;
uniform highp sampler2D inputTex_samp;
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}
void main_inner(uvec3 WorkGroupID, uvec3 LocalInvocationID, uint tint_local_index) {
  {
    uint v_3 = 0u;
    v_3 = tint_local_index;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 1024u)) {
        break;
      }
      tile[(v_4 / 256u)][(v_4 % 256u)] = vec3(0.0f);
      {
        v_3 = (v_4 + 64u);
      }
      continue;
    }
  }
  barrier();
  uint filterOffset = tint_div_u32((v.inner.filterDim - 1u), 2u);
  uint v_5 = (v_2.inner.tint_builtin_value_0 - 1u);
  uvec2 dims = uvec2(textureSize(inputTex_samp, int(min(uint(0), v_5))));
  uvec2 v_6 = ((WorkGroupID.xy * uvec2(v.inner.blockDim, 4u)) + (LocalInvocationID.xy * uvec2(4u, 1u)));
  uvec2 baseIndex = (v_6 - uvec2(filterOffset, 0u));
  {
    uint r = 0u;
    while(true) {
      if ((r < 4u)) {
      } else {
        break;
      }
      {
        uint c = 0u;
        while(true) {
          if ((c < 4u)) {
          } else {
            break;
          }
          uvec2 loadIndex = (baseIndex + uvec2(c, r));
          if ((v_1.inner.value != 0u)) {
            loadIndex = loadIndex.yx;
          }
          uint v_7 = min(r, 3u);
          uint v_8 = min(((4u * LocalInvocationID.x) + c), 255u);
          vec2 v_9 = (vec2(loadIndex) + vec2(0.25f));
          tile[v_7][v_8] = textureLod(inputTex_samp, (v_9 / vec2(dims)), 0.0f).xyz;
          {
            c = (c + 1u);
          }
          continue;
        }
      }
      {
        r = (r + 1u);
      }
      continue;
    }
  }
  barrier();
  {
    uint r = 0u;
    while(true) {
      if ((r < 4u)) {
      } else {
        break;
      }
      {
        uint c = 0u;
        while(true) {
          if ((c < 4u)) {
          } else {
            break;
          }
          uvec2 writeIndex = (baseIndex + uvec2(c, r));
          if ((v_1.inner.value != 0u)) {
            writeIndex = writeIndex.yx;
          }
          uint center = ((4u * LocalInvocationID.x) + c);
          bool v_10 = false;
          if ((center >= filterOffset)) {
            v_10 = (center < (256u - filterOffset));
          } else {
            v_10 = false;
          }
          bool v_11 = false;
          if (v_10) {
            v_11 = all(lessThan(writeIndex, dims));
          } else {
            v_11 = false;
          }
          if (v_11) {
            vec3 acc = vec3(0.0f);
            {
              uvec2 tint_loop_idx = uvec2(0u);
              uint f = 0u;
              while(true) {
                if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
                  break;
                }
                if ((f < v.inner.filterDim)) {
                } else {
                  break;
                }
                uint i = ((center + f) - filterOffset);
                vec3 v_12 = acc;
                float v_13 = (1.0f / float(v.inner.filterDim));
                uint v_14 = min(r, 3u);
                uint v_15 = min(i, 255u);
                acc = (v_12 + (v_13 * tile[v_14][v_15]));
                {
                  uint tint_low_inc = (tint_loop_idx.x + 1u);
                  tint_loop_idx.x = tint_low_inc;
                  uint tint_carry = uint((tint_low_inc == 0u));
                  tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
                  f = (f + 1u);
                }
                continue;
              }
            }
            uvec2 v_16 = writeIndex;
            vec4 v_17 = vec4(acc, 1.0f);
            imageStore(outputTex, ivec2(v_16), v_17);
          }
          {
            c = (c + 1u);
          }
          continue;
        }
      }
      {
        r = (r + 1u);
      }
      continue;
    }
  }
}
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_WorkGroupID, gl_LocalInvocationID, gl_LocalInvocationIndex);
}
