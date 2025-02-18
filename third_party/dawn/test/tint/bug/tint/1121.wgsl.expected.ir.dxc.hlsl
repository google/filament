struct main_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


RWByteAddressBuffer lightsBuffer : register(u0);
RWByteAddressBuffer tileLightId : register(u0, space1);
cbuffer cbuffer_config : register(b0, space2) {
  uint4 config[2];
};
cbuffer cbuffer_uniforms : register(b0, space3) {
  uint4 uniforms[11];
};
float4x4 v(uint start_byte_offset) {
  return float4x4(asfloat(uniforms[(start_byte_offset / 16u)]), asfloat(uniforms[((16u + start_byte_offset) / 16u)]), asfloat(uniforms[((32u + start_byte_offset) / 16u)]), asfloat(uniforms[((48u + start_byte_offset) / 16u)]));
}

void main_inner(uint3 GlobalInvocationID) {
  uint index = GlobalInvocationID.x;
  if ((index >= config[0u].x)) {
    return;
  }
  uint v_1 = 0u;
  lightsBuffer.GetDimensions(v_1);
  uint v_2 = (min(index, ((v_1 / 32u) - 1u)) * 32u);
  uint v_3 = 0u;
  lightsBuffer.GetDimensions(v_3);
  float v_4 = (asfloat(lightsBuffer.Load((4u + (min(index, ((v_3 / 32u) - 1u)) * 32u)))) - 0.10000000149011611938f);
  float v_5 = float(index);
  lightsBuffer.Store((4u + v_2), asuint((v_4 + (0.00100000004749745131f * (v_5 - (64.0f * floor((float(index) / 64.0f))))))));
  uint v_6 = 0u;
  lightsBuffer.GetDimensions(v_6);
  if ((asfloat(lightsBuffer.Load((4u + (min(index, ((v_6 / 32u) - 1u)) * 32u)))) < asfloat(uniforms[0u].y))) {
    uint v_7 = 0u;
    lightsBuffer.GetDimensions(v_7);
    lightsBuffer.Store((4u + (min(index, ((v_7 / 32u) - 1u)) * 32u)), asuint(asfloat(uniforms[1u].y)));
  }
  float4x4 M = v(96u);
  float viewNear = (-(M[3u].z) / (-1.0f + M[2u].z));
  float viewFar = (-(M[3u].z) / (1.0f + M[2u].z));
  uint v_8 = 0u;
  lightsBuffer.GetDimensions(v_8);
  float4 lightPos = asfloat(lightsBuffer.Load4((0u + (min(index, ((v_8 / 32u) - 1u)) * 32u))));
  float4x4 v_9 = v(32u);
  lightPos = mul(lightPos, v_9);
  lightPos = (lightPos / lightPos.w);
  uint v_10 = 0u;
  lightsBuffer.GetDimensions(v_10);
  float lightRadius = asfloat(lightsBuffer.Load((28u + (min(index, ((v_10 / 32u) - 1u)) * 32u))));
  float4 v_11 = lightPos;
  float4 boxMin = (v_11 - float4(float3((lightRadius).xxx), 0.0f));
  float4 v_12 = lightPos;
  float4 boxMax = (v_12 + float4(float3((lightRadius).xxx), 0.0f));
  float4 frustumPlanes[6] = (float4[6])0;
  frustumPlanes[4u] = float4(0.0f, 0.0f, -1.0f, viewNear);
  frustumPlanes[5u] = float4(0.0f, 0.0f, 1.0f, -(viewFar));
  int TILE_SIZE = int(16);
  int TILE_COUNT_X = int(2);
  int TILE_COUNT_Y = int(2);
  {
    int y = int(0);
    while(true) {
      if ((y < TILE_COUNT_Y)) {
      } else {
        break;
      }
      {
        int x = int(0);
        while(true) {
          if ((x < TILE_COUNT_X)) {
          } else {
            break;
          }
          int2 tilePixel0Idx = int2((x * TILE_SIZE), (y * TILE_SIZE));
          float2 v_13 = (2.0f * float2(tilePixel0Idx));
          float2 floorCoord = ((v_13 / asfloat(uniforms[10u]).xy) - (1.0f).xx);
          int2 v_14 = tilePixel0Idx;
          float2 v_15 = (2.0f * float2((v_14 + int2((TILE_SIZE).xx))));
          float2 ceilCoord = ((v_15 / asfloat(uniforms[10u]).xy) - (1.0f).xx);
          float2 viewFloorCoord = float2((((-(viewNear) * floorCoord.x) - (M[2u].x * viewNear)) / M[0u].x), (((-(viewNear) * floorCoord.y) - (M[2u].y * viewNear)) / M[1u].y));
          float2 viewCeilCoord = float2((((-(viewNear) * ceilCoord.x) - (M[2u].x * viewNear)) / M[0u].x), (((-(viewNear) * ceilCoord.y) - (M[2u].y * viewNear)) / M[1u].y));
          frustumPlanes[0u] = float4(1.0f, 0.0f, (-(viewFloorCoord.x) / viewNear), 0.0f);
          frustumPlanes[1u] = float4(-1.0f, 0.0f, (viewCeilCoord.x / viewNear), 0.0f);
          frustumPlanes[2u] = float4(0.0f, 1.0f, (-(viewFloorCoord.y) / viewNear), 0.0f);
          frustumPlanes[3u] = float4(0.0f, -1.0f, (viewCeilCoord.y / viewNear), 0.0f);
          float dp = 0.0f;
          {
            uint i = 0u;
            while(true) {
              if ((i < 6u)) {
              } else {
                break;
              }
              float4 p = (0.0f).xxxx;
              uint v_16 = min(i, 5u);
              if ((frustumPlanes[v_16].x > 0.0f)) {
                p.x = boxMax.x;
              } else {
                p.x = boxMin.x;
              }
              uint v_17 = min(i, 5u);
              if ((frustumPlanes[v_17].y > 0.0f)) {
                p.y = boxMax.y;
              } else {
                p.y = boxMin.y;
              }
              uint v_18 = min(i, 5u);
              if ((frustumPlanes[v_18].z > 0.0f)) {
                p.z = boxMax.z;
              } else {
                p.z = boxMin.z;
              }
              p.w = 1.0f;
              float v_19 = dp;
              float4 v_20 = p;
              uint v_21 = min(i, 5u);
              dp = (v_19 + min(0.0f, dot(v_20, frustumPlanes[v_21])));
              {
                i = (i + 1u);
              }
              continue;
            }
          }
          if ((dp >= 0.0f)) {
            uint tileId = uint((x + (y * TILE_COUNT_X)));
            bool v_22 = false;
            if ((tileId < 0u)) {
              v_22 = true;
            } else {
              v_22 = (tileId >= config[0u].y);
            }
            if (v_22) {
              {
                x = (x + int(1));
              }
              continue;
            }
            uint v_23 = 0u;
            tileLightId.InterlockedAdd((0u + (min(tileId, 3u) * 260u)), 1u, v_23);
            uint offset = v_23;
            if ((offset >= config[1u].x)) {
              {
                x = (x + int(1));
              }
              continue;
            }
            tileLightId.Store(((4u + (min(tileId, 3u) * 260u)) + (min(offset, 63u) * 4u)), GlobalInvocationID.x);
          }
          {
            x = (x + int(1));
          }
          continue;
        }
      }
      {
        y = (y + int(1));
      }
      continue;
    }
  }
}

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationID);
}

