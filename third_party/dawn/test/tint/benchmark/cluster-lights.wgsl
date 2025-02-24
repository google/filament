struct Camera {
  projection : mat4x4<f32>,
  inverseProjection : mat4x4<f32>,
  view : mat4x4<f32>,
  position : vec3<f32>,
  time : f32,
  outputSize : vec2<f32>,
  zNear : f32,
  zFar : f32,
}

@group(0) @binding(0) var<uniform> camera : Camera;

struct ClusterBounds {
  minAABB : vec3<f32>,
  maxAABB : vec3<f32>,
}

struct Clusters {
  bounds : array<ClusterBounds, 27648>,
}

@group(0) @binding(1) var<storage, read> clusters : Clusters;

struct ClusterLights {
  offset : u32,
  count : u32,
}

struct ClusterLightGroup {
  offset : atomic<u32>,
  lights : array<ClusterLights, 27648>,
  indices : array<u32, 1769472>,
}

@group(0) @binding(2) var<storage, read_write> clusterLights : ClusterLightGroup;

struct Light {
  position : vec3<f32>,
  range : f32,
  color : vec3<f32>,
  intensity : f32,
}

struct GlobalLights {
  ambient : vec3<f32>,
  dirColor : vec3<f32>,
  dirIntensity : f32,
  dirDirection : vec3<f32>,
  lightCount : u32,
  lights : array<Light>,
}

@group(0) @binding(3) var<storage, read> globalLights : GlobalLights;

const tileCount = vec3(32u, 18u, 48u);

fn linearDepth(depthSample : f32) -> f32 {
  return ((camera.zFar * camera.zNear) / fma(depthSample, (camera.zNear - camera.zFar), camera.zFar));
}

fn getTile(fragCoord : vec4<f32>) -> vec3<u32> {
  let sliceScale = (f32(tileCount.z) / log2((camera.zFar / camera.zNear)));
  let sliceBias = -(((f32(tileCount.z) * log2(camera.zNear)) / log2((camera.zFar / camera.zNear))));
  let zTile = u32(max(((log2(linearDepth(fragCoord.z)) * sliceScale) + sliceBias), 0.0));
  return vec3(u32((fragCoord.x / (camera.outputSize.x / f32(tileCount.x)))), u32((fragCoord.y / (camera.outputSize.y / f32(tileCount.y)))), zTile);
}

fn getClusterIndex(fragCoord : vec4<f32>) -> u32 {
  let tile = getTile(fragCoord);
  return ((tile.x + (tile.y * tileCount.x)) + ((tile.z * tileCount.x) * tileCount.y));
}

fn sqDistPointAABB(p: vec3<f32>, minAABB : vec3<f32>, maxAABB : vec3<f32>) -> f32 {
  var sqDist = 0.0;
  for(var i : i32 = 0; (i < 3); i = (i + 1)) {
    let v = p[i];
    if ((v < minAABB[i])) {
      sqDist = (sqDist + ((minAABB[i] - v) * (minAABB[i] - v)));
    }
    if ((v > maxAABB[i])) {
      sqDist = (sqDist + ((v - maxAABB[i]) * (v - maxAABB[i])));
    }
  }
  return sqDist;
}

@compute @workgroup_size(4, 2, 4)
fn computeMain(@builtin(global_invocation_id) global_id : vec3<u32>) {
  let tileIndex = ((global_id.x + (global_id.y * tileCount.x)) + ((global_id.z * tileCount.x) * tileCount.y));
  var clusterLightCount = 0u;
  var cluserLightIndices : array<u32, 256>;
  for(var i = 0u; (i < globalLights.lightCount); i = (i + 1u)) {
    let range = globalLights.lights[i].range;
    var lightInCluster : bool = (range <= 0.0);
    if (!(lightInCluster)) {
      let lightViewPos = (camera.view * vec4(globalLights.lights[i].position, 1.0));
      let sqDist = sqDistPointAABB(lightViewPos.xyz, clusters.bounds[tileIndex].minAABB, clusters.bounds[tileIndex].maxAABB);
      lightInCluster = (sqDist <= (range * range));
    }
    if (lightInCluster) {
      cluserLightIndices[clusterLightCount] = i;
      clusterLightCount = (clusterLightCount + 1u);
    }
    if ((clusterLightCount == 256u)) {
      break;
    }
  }
  let lightCount = clusterLightCount;
  var offset = atomicAdd(&(clusterLights.offset), lightCount);
  if ((offset >= 1769472u)) {
    return;
  }
  for(var i = 0u; (i < clusterLightCount); i = (i + 1u)) {
    clusterLights.indices[(offset + i)] = cluserLightIndices[i];
  }
  clusterLights.lights[tileIndex].offset = offset;
  clusterLights.lights[tileIndex].count = clusterLightCount;
}
