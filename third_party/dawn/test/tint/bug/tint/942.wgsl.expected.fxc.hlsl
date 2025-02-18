groupshared float3 tile[4][256];

void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 1024u); idx = (idx + 64u)) {
      uint i_1 = (idx / 256u);
      uint i_2 = (idx % 256u);
      tile[i_1][i_2] = (0.0f).xxx;
    }
  }
  GroupMemoryBarrierWithGroupSync();
}

SamplerState samp : register(s0);
cbuffer cbuffer_params : register(b1) {
  uint4 params[1];
};
Texture2D<float4> inputTex : register(t1, space1);
RWTexture2D<float4> outputTex : register(u2, space1);

cbuffer cbuffer_flip : register(b3, space1) {
  uint4 flip[1];
};

uint tint_div(uint lhs, uint rhs) {
  return (lhs / ((rhs == 0u) ? 1u : rhs));
}

struct tint_symbol_1 {
  uint3 LocalInvocationID : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
  uint3 WorkGroupID : SV_GroupID;
};

void main_inner(uint3 WorkGroupID, uint3 LocalInvocationID, uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  uint filterOffset = tint_div((params[0].x - 1u), 2u);
  uint3 tint_tmp;
  inputTex.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  uint2 dims = tint_tmp.xy;
  uint2 baseIndex = (((WorkGroupID.xy * uint2(params[0].y, 4u)) + (LocalInvocationID.xy * uint2(4u, 1u))) - uint2(filterOffset, 0u));
  {
    for(uint r = 0u; (r < 4u); r = (r + 1u)) {
      {
        for(uint c = 0u; (c < 4u); c = (c + 1u)) {
          uint2 loadIndex = (baseIndex + uint2(c, r));
          if ((flip[0].x != 0u)) {
            loadIndex = loadIndex.yx;
          }
          tile[min(r, 3u)][min(((4u * LocalInvocationID.x) + c), 255u)] = inputTex.SampleLevel(samp, ((float2(loadIndex) + (0.25f).xx) / float2(dims)), 0.0f).rgb;
        }
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  {
    for(uint r = 0u; (r < 4u); r = (r + 1u)) {
      {
        for(uint c = 0u; (c < 4u); c = (c + 1u)) {
          uint2 writeIndex = (baseIndex + uint2(c, r));
          if ((flip[0].x != 0u)) {
            writeIndex = writeIndex.yx;
          }
          uint center = ((4u * LocalInvocationID.x) + c);
          bool tint_tmp_2 = (center >= filterOffset);
          if (tint_tmp_2) {
            tint_tmp_2 = (center < (256u - filterOffset));
          }
          bool tint_tmp_1 = (tint_tmp_2);
          if (tint_tmp_1) {
            tint_tmp_1 = all((writeIndex < dims));
          }
          if ((tint_tmp_1)) {
            float3 acc = (0.0f).xxx;
            {
              for(uint f = 0u; (f < params[0].x); f = (f + 1u)) {
                uint i = ((center + f) - filterOffset);
                acc = (acc + ((1.0f / float(params[0].x)) * tile[min(r, 3u)][min(i, 255u)]));
              }
            }
            outputTex[writeIndex] = float4(acc, 1.0f);
          }
        }
      }
    }
  }
}

[numthreads(64, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.WorkGroupID, tint_symbol.LocalInvocationID, tint_symbol.local_invocation_index);
  return;
}
