#version 310 es


struct LightData {
  vec4 position;
  vec3 color;
  float radius;
};

struct TileLightIdData {
  uint count;
  uint lightId[64];
};

struct Tiles {
  TileLightIdData data[4];
};

struct Config {
  uint numLights;
  uint numTiles;
  uint tileCountX;
  uint tileCountY;
  uint numTileLightSlot;
  uint tileSize;
};

struct Uniforms {
  vec4 member_0;
  vec4 member_1;
  mat4 viewMatrix;
  mat4 projectionMatrix;
  vec4 fullScreenSize;
};

layout(binding = 0, std430)
buffer LightsBuffer_1_ssbo {
  LightData lights[];
} lightsBuffer;
layout(binding = 0, std430)
buffer tileLightId_block_1_ssbo {
  Tiles inner;
} v;
layout(binding = 0, std140)
uniform config_block_1_ubo {
  Config inner;
} v_1;
layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v_2;
void main_inner(uvec3 GlobalInvocationID) {
  uint index = GlobalInvocationID.x;
  if ((index >= v_1.inner.numLights)) {
    return;
  }
  uint v_3 = index;
  uint v_4 = min(v_3, (uint(lightsBuffer.lights.length()) - 1u));
  uint v_5 = index;
  uint v_6 = min(v_5, (uint(lightsBuffer.lights.length()) - 1u));
  float v_7 = (lightsBuffer.lights[v_6].position.y - 0.10000000149011611938f);
  float v_8 = float(index);
  lightsBuffer.lights[v_4].position.y = (v_7 + (0.00100000004749745131f * (v_8 - (64.0f * floor((float(index) / 64.0f))))));
  uint v_9 = index;
  uint v_10 = min(v_9, (uint(lightsBuffer.lights.length()) - 1u));
  if ((lightsBuffer.lights[v_10].position.y < v_2.inner.member_0.y)) {
    uint v_11 = index;
    uint v_12 = min(v_11, (uint(lightsBuffer.lights.length()) - 1u));
    lightsBuffer.lights[v_12].position.y = v_2.inner.member_1.y;
  }
  mat4 M = v_2.inner.projectionMatrix;
  float viewNear = (-(M[3u].z) / (-1.0f + M[2u].z));
  float viewFar = (-(M[3u].z) / (1.0f + M[2u].z));
  uint v_13 = index;
  uint v_14 = min(v_13, (uint(lightsBuffer.lights.length()) - 1u));
  vec4 lightPos = lightsBuffer.lights[v_14].position;
  lightPos = (v_2.inner.viewMatrix * lightPos);
  lightPos = (lightPos / lightPos.w);
  uint v_15 = index;
  uint v_16 = min(v_15, (uint(lightsBuffer.lights.length()) - 1u));
  float lightRadius = lightsBuffer.lights[v_16].radius;
  vec4 v_17 = lightPos;
  vec4 boxMin = (v_17 - vec4(vec3(lightRadius), 0.0f));
  vec4 v_18 = lightPos;
  vec4 boxMax = (v_18 + vec4(vec3(lightRadius), 0.0f));
  vec4 frustumPlanes[6] = vec4[6](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  frustumPlanes[4u] = vec4(0.0f, 0.0f, -1.0f, viewNear);
  frustumPlanes[5u] = vec4(0.0f, 0.0f, 1.0f, -(viewFar));
  int TILE_SIZE = 16;
  int TILE_COUNT_X = 2;
  int TILE_COUNT_Y = 2;
  {
    int y = 0;
    while(true) {
      if ((y < TILE_COUNT_Y)) {
      } else {
        break;
      }
      {
        int x = 0;
        while(true) {
          if ((x < TILE_COUNT_X)) {
          } else {
            break;
          }
          ivec2 tilePixel0Idx = ivec2((x * TILE_SIZE), (y * TILE_SIZE));
          vec2 v_19 = (2.0f * vec2(tilePixel0Idx));
          vec2 floorCoord = ((v_19 / v_2.inner.fullScreenSize.xy) - vec2(1.0f));
          ivec2 v_20 = tilePixel0Idx;
          vec2 v_21 = (2.0f * vec2((v_20 + ivec2(TILE_SIZE))));
          vec2 ceilCoord = ((v_21 / v_2.inner.fullScreenSize.xy) - vec2(1.0f));
          vec2 viewFloorCoord = vec2((((-(viewNear) * floorCoord.x) - (M[2u].x * viewNear)) / M[0u].x), (((-(viewNear) * floorCoord.y) - (M[2u].y * viewNear)) / M[1u].y));
          vec2 viewCeilCoord = vec2((((-(viewNear) * ceilCoord.x) - (M[2u].x * viewNear)) / M[0u].x), (((-(viewNear) * ceilCoord.y) - (M[2u].y * viewNear)) / M[1u].y));
          frustumPlanes[0u] = vec4(1.0f, 0.0f, (-(viewFloorCoord.x) / viewNear), 0.0f);
          frustumPlanes[1u] = vec4(-1.0f, 0.0f, (viewCeilCoord.x / viewNear), 0.0f);
          frustumPlanes[2u] = vec4(0.0f, 1.0f, (-(viewFloorCoord.y) / viewNear), 0.0f);
          frustumPlanes[3u] = vec4(0.0f, -1.0f, (viewCeilCoord.y / viewNear), 0.0f);
          float dp = 0.0f;
          {
            uint i = 0u;
            while(true) {
              if ((i < 6u)) {
              } else {
                break;
              }
              vec4 p = vec4(0.0f);
              uint v_22 = min(i, 5u);
              if ((frustumPlanes[v_22].x > 0.0f)) {
                p.x = boxMax.x;
              } else {
                p.x = boxMin.x;
              }
              uint v_23 = min(i, 5u);
              if ((frustumPlanes[v_23].y > 0.0f)) {
                p.y = boxMax.y;
              } else {
                p.y = boxMin.y;
              }
              uint v_24 = min(i, 5u);
              if ((frustumPlanes[v_24].z > 0.0f)) {
                p.z = boxMax.z;
              } else {
                p.z = boxMin.z;
              }
              p.w = 1.0f;
              float v_25 = dp;
              vec4 v_26 = p;
              uint v_27 = min(i, 5u);
              dp = (v_25 + min(0.0f, dot(v_26, frustumPlanes[v_27])));
              {
                i = (i + 1u);
              }
              continue;
            }
          }
          if ((dp >= 0.0f)) {
            uint tileId = uint((x + (y * TILE_COUNT_X)));
            bool v_28 = false;
            if ((tileId < 0u)) {
              v_28 = true;
            } else {
              v_28 = (tileId >= v_1.inner.numTiles);
            }
            if (v_28) {
              {
                x = (x + 1);
              }
              continue;
            }
            uint v_29 = min(tileId, 3u);
            uint offset = atomicAdd(v.inner.data[v_29].count, 1u);
            if ((offset >= v_1.inner.numTileLightSlot)) {
              {
                x = (x + 1);
              }
              continue;
            }
            uint v_30 = min(tileId, 3u);
            uint v_31 = min(offset, 63u);
            v.inner.data[v_30].lightId[v_31] = GlobalInvocationID.x;
          }
          {
            x = (x + 1);
          }
          continue;
        }
      }
      {
        y = (y + 1);
      }
      continue;
    }
  }
}
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_GlobalInvocationID);
}
