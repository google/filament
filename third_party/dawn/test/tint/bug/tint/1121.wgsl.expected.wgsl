struct LightData {
  position : vec4<f32>,
  color : vec3<f32>,
  radius : f32,
}

struct LightsBuffer {
  lights : array<LightData>,
}

@group(0) @binding(0) var<storage, read_write> lightsBuffer : LightsBuffer;

struct TileLightIdData {
  count : atomic<u32>,
  lightId : array<u32, 64>,
}

struct Tiles {
  data : array<TileLightIdData, 4>,
}

@group(1) @binding(0) var<storage, read_write> tileLightId : Tiles;

struct Config {
  numLights : u32,
  numTiles : u32,
  tileCountX : u32,
  tileCountY : u32,
  numTileLightSlot : u32,
  tileSize : u32,
}

@group(2) @binding(0) var<uniform> config : Config;

struct Uniforms {
  min : vec4<f32>,
  max : vec4<f32>,
  viewMatrix : mat4x4<f32>,
  projectionMatrix : mat4x4<f32>,
  fullScreenSize : vec4<f32>,
}

@group(3) @binding(0) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(64, 1, 1)
fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
  var index = GlobalInvocationID.x;
  if ((index >= config.numLights)) {
    return;
  }
  lightsBuffer.lights[index].position.y = ((lightsBuffer.lights[index].position.y - 0.10000000000000000555) + (0.00100000000000000002 * (f32(index) - (64.0 * floor((f32(index) / 64.0))))));
  if ((lightsBuffer.lights[index].position.y < uniforms.min.y)) {
    lightsBuffer.lights[index].position.y = uniforms.max.y;
  }
  var M : mat4x4<f32> = uniforms.projectionMatrix;
  var viewNear : f32 = (-(M[3][2]) / (-(1.0) + M[2][2]));
  var viewFar : f32 = (-(M[3][2]) / (1.0 + M[2][2]));
  var lightPos = lightsBuffer.lights[index].position;
  lightPos = (uniforms.viewMatrix * lightPos);
  lightPos = (lightPos / lightPos.w);
  var lightRadius : f32 = lightsBuffer.lights[index].radius;
  var boxMin : vec4<f32> = (lightPos - vec4<f32>(vec3<f32>(lightRadius), 0.0));
  var boxMax : vec4<f32> = (lightPos + vec4<f32>(vec3<f32>(lightRadius), 0.0));
  var frustumPlanes : array<vec4<f32>, 6>;
  frustumPlanes[4] = vec4<f32>(0.0, 0.0, -(1.0), viewNear);
  frustumPlanes[5] = vec4<f32>(0.0, 0.0, 1.0, -(viewFar));
  let TILE_SIZE : i32 = 16;
  let TILE_COUNT_X : i32 = 2;
  let TILE_COUNT_Y : i32 = 2;
  for(var y : i32 = 0; (y < TILE_COUNT_Y); y = (y + 1)) {
    for(var x : i32 = 0; (x < TILE_COUNT_X); x = (x + 1)) {
      var tilePixel0Idx : vec2<i32> = vec2<i32>((x * TILE_SIZE), (y * TILE_SIZE));
      var floorCoord : vec2<f32> = (((2.0 * vec2<f32>(tilePixel0Idx)) / uniforms.fullScreenSize.xy) - vec2<f32>(1.0));
      var ceilCoord : vec2<f32> = (((2.0 * vec2<f32>((tilePixel0Idx + vec2<i32>(TILE_SIZE)))) / uniforms.fullScreenSize.xy) - vec2<f32>(1.0));
      var viewFloorCoord : vec2<f32> = vec2<f32>((((-(viewNear) * floorCoord.x) - (M[2][0] * viewNear)) / M[0][0]), (((-(viewNear) * floorCoord.y) - (M[2][1] * viewNear)) / M[1][1]));
      var viewCeilCoord : vec2<f32> = vec2<f32>((((-(viewNear) * ceilCoord.x) - (M[2][0] * viewNear)) / M[0][0]), (((-(viewNear) * ceilCoord.y) - (M[2][1] * viewNear)) / M[1][1]));
      frustumPlanes[0] = vec4<f32>(1.0, 0.0, (-(viewFloorCoord.x) / viewNear), 0.0);
      frustumPlanes[1] = vec4<f32>(-(1.0), 0.0, (viewCeilCoord.x / viewNear), 0.0);
      frustumPlanes[2] = vec4<f32>(0.0, 1.0, (-(viewFloorCoord.y) / viewNear), 0.0);
      frustumPlanes[3] = vec4<f32>(0.0, -(1.0), (viewCeilCoord.y / viewNear), 0.0);
      var dp : f32 = 0.0;
      for(var i : u32 = 0u; (i < 6u); i = (i + 1u)) {
        var p : vec4<f32>;
        if ((frustumPlanes[i].x > 0.0)) {
          p.x = boxMax.x;
        } else {
          p.x = boxMin.x;
        }
        if ((frustumPlanes[i].y > 0.0)) {
          p.y = boxMax.y;
        } else {
          p.y = boxMin.y;
        }
        if ((frustumPlanes[i].z > 0.0)) {
          p.z = boxMax.z;
        } else {
          p.z = boxMin.z;
        }
        p.w = 1.0;
        dp = (dp + min(0.0, dot(p, frustumPlanes[i])));
      }
      if ((dp >= 0.0)) {
        var tileId : u32 = u32((x + (y * TILE_COUNT_X)));
        if (((tileId < 0u) || (tileId >= config.numTiles))) {
          continue;
        }
        var offset : u32 = atomicAdd(&(tileLightId.data[tileId].count), 1u);
        if ((offset >= config.numTileLightSlot)) {
          continue;
        }
        tileLightId.data[tileId].lightId[offset] = GlobalInvocationID.x;
      }
    }
  }
}
