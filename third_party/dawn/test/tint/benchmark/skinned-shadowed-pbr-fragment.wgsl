const GAMMA = 2.200000048;

fn linearTosRGB(linear : vec3<f32>) -> vec3<f32> {
  let INV_GAMMA = (1.0 / GAMMA);
  return pow(linear, vec3(INV_GAMMA));
}

fn sRGBToLinear(srgb : vec3<f32>) -> vec3<f32> {
  return pow(srgb, vec3(GAMMA));
}

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

@binding(0) @group(0) var<uniform> camera : Camera;

struct ClusterLights {
  offset : u32,
  count : u32,
}

struct ClusterLightGroup {
  offset : u32,
  lights : array<ClusterLights, 27648>,
  indices : array<u32, 1769472>,
}

@binding(1) @group(0) var<storage, read> clusterLights : ClusterLightGroup;

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

@binding(2) @group(0) var<storage, read> globalLights : GlobalLights;

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

@binding(3) @group(0) var defaultSampler : sampler;

@binding(4) @group(0) var shadowTexture : texture_depth_2d;

@binding(5) @group(0) var shadowSampler : sampler_comparison;

struct LightShadowTable {
  light : array<i32>,
}

@binding(6) @group(0) var<storage, read> lightShadowTable : LightShadowTable;

var<private> shadowSampleOffsets : array<vec2<f32>, 16> = array<vec2<f32>, 16>(vec2(-1.5, -1.5), vec2(-1.5, -0.5), vec2(-1.5, 0.5), vec2(-1.5, 1.5), vec2(-0.5, -1.5), vec2(-0.5, -0.5), vec2(-0.5, 0.5), vec2(-0.5, 1.5), vec2(0.5, -1.5), vec2(0.5, -0.5), vec2(0.5, 0.5), vec2(0.5, 1.5), vec2(1.5, -1.5), vec2(1.5, -0.5), vec2(1.5, 0.5), vec2(1.5, 1.5));

const shadowSampleCount = 16u;

struct ShadowProperties {
  viewport : vec4<f32>,
  viewProj : mat4x4<f32>,
}

struct LightShadows {
  properties : array<ShadowProperties>,
}

@binding(7) @group(0) var<storage, read> shadow : LightShadows;

fn dirLightVisibility(worldPos : vec3<f32>) -> f32 {
  let shadowIndex = lightShadowTable.light[0u];
  if ((shadowIndex == -1)) {
    return 1.0;
  }
  let viewport = shadow.properties[shadowIndex].viewport;
  let lightPos = (shadow.properties[shadowIndex].viewProj * vec4(worldPos, 1.0));
  let shadowPos = vec3((((lightPos.xy / lightPos.w) * vec2(0.5, -0.5)) + vec2(0.5, 0.5)), (lightPos.z / lightPos.w));
  let viewportPos = vec2((viewport.xy + (shadowPos.xy * viewport.zw)));
  let texelSize = (1.0 / vec2<f32>(textureDimensions(shadowTexture, 0)));
  let clampRect = vec4((viewport.xy - texelSize), ((viewport.xy + viewport.zw) + texelSize));
  var visibility = 0.0;
  for(var i = 0u; (i < shadowSampleCount); i = (i + 1u)) {
    visibility = (visibility + textureSampleCompareLevel(shadowTexture, shadowSampler, clamp((viewportPos + (shadowSampleOffsets[i] * texelSize)), clampRect.xy, clampRect.zw), (shadowPos.z - 0.003)));
  }
  return (visibility / f32(shadowSampleCount));
}

fn getCubeFace(v : vec3<f32>) -> i32 {
  let vAbs = abs(v);
  if (((vAbs.z >= vAbs.x) && (vAbs.z >= vAbs.y))) {
    if ((v.z < 0.0)) {
      return 5;
    }
    return 4;
  }
  if ((vAbs.y >= vAbs.x)) {
    if ((v.y < 0.0)) {
      return 3;
    }
    return 2;
  }
  if ((v.x < 0.0)) {
    return 1;
  }
  return 0;
}

fn pointLightVisibility(lightIndex : u32, worldPos : vec3<f32>, pointToLight : vec3<f32>) -> f32 {
  var shadowIndex = lightShadowTable.light[(lightIndex + 1u)];
  if ((shadowIndex == -1)) {
    return 1.0;
  }
  shadowIndex = (shadowIndex + getCubeFace((pointToLight * -1.0)));
  let viewport = shadow.properties[shadowIndex].viewport;
  let lightPos = (shadow.properties[shadowIndex].viewProj * vec4(worldPos, 1.0));
  let shadowPos = vec3((((lightPos.xy / lightPos.w) * vec2(0.5, -0.5)) + vec2(0.5, 0.5)), (lightPos.z / lightPos.w));
  let viewportPos = vec2((viewport.xy + (shadowPos.xy * viewport.zw)));
  let texelSize = (1.0 / vec2<f32>(textureDimensions(shadowTexture, 0)));
  let clampRect = vec4(viewport.xy, (viewport.xy + viewport.zw));
  var visibility = 0.0;
  for(var i = 0u; (i < shadowSampleCount); i = (i + 1u)) {
    visibility = (visibility + textureSampleCompareLevel(shadowTexture, shadowSampler, clamp((viewportPos + (shadowSampleOffsets[i] * texelSize)), clampRect.xy, clampRect.zw), (shadowPos.z - 0.01)));
  }
  return (visibility / f32(shadowSampleCount));
}

struct VertexOutput {
  @builtin(position)
  position : vec4<f32>,
  @location(0)
  worldPos : vec3<f32>,
  @location(1)
  view : vec3<f32>,
  @location(2)
  texcoord : vec2<f32>,
  @location(3)
  texcoord2 : vec2<f32>,
  @location(4)
  color : vec4<f32>,
  @location(5)
  instanceColor : vec4<f32>,
  @location(6)
  normal : vec3<f32>,
  @location(7)
  tangent : vec3<f32>,
  @location(8)
  bitangent : vec3<f32>,
}

struct Material {
  baseColorFactor : vec4<f32>,
  emissiveFactor : vec3<f32>,
  occlusionStrength : f32,
  metallicRoughnessFactor : vec2<f32>,
  alphaCutoff : f32,
}

@binding(8) @group(0) var<uniform> material : Material;

@binding(9) @group(0) var baseColorTexture : texture_2d<f32>;

@binding(10) @group(0) var baseColorSampler : sampler;

@binding(11) @group(0) var normalTexture : texture_2d<f32>;

@binding(12) @group(0) var normalSampler : sampler;

@binding(13) @group(0) var metallicRoughnessTexture : texture_2d<f32>;

@binding(14) @group(0) var metallicRoughnessSampler : sampler;

@binding(15) @group(0) var occlusionTexture : texture_2d<f32>;

@binding(16) @group(0) var occlusionSampler : sampler;

@binding(17) @group(0) var emissiveTexture : texture_2d<f32>;

@binding(18) @group(0) var emissiveSampler : sampler;

struct SurfaceInfo {
  baseColor : vec4<f32>,
  albedo : vec3<f32>,
  metallic : f32,
  roughness : f32,
  normal : vec3<f32>,
  f0 : vec3<f32>,
  ao : f32,
  emissive : vec3<f32>,
  v : vec3<f32>,
}

fn GetSurfaceInfo(input : VertexOutput) -> SurfaceInfo {
  var surface : SurfaceInfo;
  surface.v = normalize(input.view);
  let tbn = mat3x3(input.tangent, input.bitangent, input.normal);
  let normalMap = textureSample(normalTexture, normalSampler, input.texcoord).rgb;
  surface.normal = normalize((tbn * ((2.0 * normalMap) - vec3(1.0))));
  let baseColorMap = textureSample(baseColorTexture, baseColorSampler, input.texcoord);
  surface.baseColor = ((input.color * material.baseColorFactor) * baseColorMap);
  if ((surface.baseColor.a < material.alphaCutoff)) {
    // Violates uniformity analysis:
    // discard;
  }
  surface.albedo = surface.baseColor.rgb;
  let metallicRoughnessMap = textureSample(metallicRoughnessTexture, metallicRoughnessSampler, input.texcoord);
  surface.metallic = (material.metallicRoughnessFactor.x * metallicRoughnessMap.b);
  surface.roughness = (material.metallicRoughnessFactor.y * metallicRoughnessMap.g);
  let dielectricSpec = vec3(0.039999999);
  surface.f0 = mix(dielectricSpec, surface.albedo, vec3(surface.metallic));
  let occlusionMap = textureSample(occlusionTexture, occlusionSampler, input.texcoord);
  surface.ao = (material.occlusionStrength * occlusionMap.r);
  let emissiveMap = textureSample(emissiveTexture, emissiveSampler, input.texcoord);
  surface.emissive = (material.emissiveFactor * emissiveMap.rgb);
  if ((input.instanceColor.a == 0.0)) {
    surface.albedo = (surface.albedo + input.instanceColor.rgb);
  } else {
    surface.albedo = (surface.albedo * input.instanceColor.rgb);
  }
  return surface;
}

const PI = 3.141592741;

const LightType_Point = 0u;

const LightType_Spot = 1u;

const LightType_Directional = 2u;

struct PuctualLight {
  lightType : u32,
  pointToLight : vec3<f32>,
  range : f32,
  color : vec3<f32>,
  intensity : f32,
}

fn FresnelSchlick(cosTheta : f32, F0 : vec3<f32>) -> vec3<f32> {
  return (F0 + ((vec3(1.0) - F0) * pow((1.0 - cosTheta), 5.0)));
}

fn DistributionGGX(N : vec3<f32>, H : vec3<f32>, roughness : f32) -> f32 {
  let a = (roughness * roughness);
  let a2 = (a * a);
  let NdotH = max(dot(N, H), 0.0);
  let NdotH2 = (NdotH * NdotH);
  let num = a2;
  let denom = ((NdotH2 * (a2 - 1.0)) + 1.0);
  return (num / ((PI * denom) * denom));
}

fn GeometrySchlickGGX(NdotV : f32, roughness : f32) -> f32 {
  let r = (roughness + 1.0);
  let k = ((r * r) / 8.0);
  let num = NdotV;
  let denom = ((NdotV * (1.0 - k)) + k);
  return (num / denom);
}

fn GeometrySmith(N : vec3<f32>, V : vec3<f32>, L : vec3<f32>, roughness : f32) -> f32 {
  let NdotV = max(dot(N, V), 0.0);
  let NdotL = max(dot(N, L), 0.0);
  let ggx2 = GeometrySchlickGGX(NdotV, roughness);
  let ggx1 = GeometrySchlickGGX(NdotL, roughness);
  return (ggx1 * ggx2);
}

fn lightAttenuation(light : PuctualLight) -> f32 {
  if ((light.lightType == LightType_Directional)) {
    return 1.0;
  }
  let distance = length(light.pointToLight);
  if ((light.range <= 0.0)) {
    return (1.0 / pow(distance, 2.0));
  }
  return (clamp((1.0 - pow((distance / light.range), 4.0)), 0.0, 1.0) / pow(distance, 2.0));
}

fn lightRadiance(light : PuctualLight, surface : SurfaceInfo) -> vec3<f32> {
  let L = normalize(light.pointToLight);
  let H = normalize((surface.v + L));
  let NDF = DistributionGGX(surface.normal, H, surface.roughness);
  let G = GeometrySmith(surface.normal, surface.v, L, surface.roughness);
  let F = FresnelSchlick(max(dot(H, surface.v), 0.0), surface.f0);
  let kD = ((vec3(1.0) - F) * (1.0 - surface.metallic));
  let NdotL = max(dot(surface.normal, L), 0.0);
  let numerator = ((NDF * G) * F);
  let denominator = max(((4.0 * max(dot(surface.normal, surface.v), 0.0)) * NdotL), 0.001);
  let specular = (numerator / vec3(denominator));
  let radiance = ((light.color * light.intensity) * lightAttenuation(light));
  return (((((kD * surface.albedo) / vec3(PI)) + specular) * radiance) * NdotL);
}

@binding(19) @group(0) var ssaoTexture : texture_2d<f32>;

struct FragmentOutput {
  @location(0)
  color : vec4<f32>,
  @location(1)
  emissive : vec4<f32>,
}

@fragment
fn fragmentMain(input : VertexOutput) -> FragmentOutput {
  let surface = GetSurfaceInfo(input);
  var Lo = vec3(0.0, 0.0, 0.0);
  if ((globalLights.dirIntensity > 0.0)) {
    var light : PuctualLight;
    light.lightType = LightType_Directional;
    light.pointToLight = globalLights.dirDirection;
    light.color = globalLights.dirColor;
    light.intensity = globalLights.dirIntensity;
    let lightVis = dirLightVisibility(input.worldPos);
    Lo = (Lo + (lightRadiance(light, surface) * lightVis));
  }
  let clusterIndex = getClusterIndex(input.position);
  let lightOffset = clusterLights.lights[clusterIndex].offset;
  let lightCount = clusterLights.lights[clusterIndex].count;
  for(var lightIndex = 0u; (lightIndex < lightCount); lightIndex = (lightIndex + 1u)) {
    let i = clusterLights.indices[(lightOffset + lightIndex)];
    var light : PuctualLight;
    light.lightType = LightType_Point;
    light.pointToLight = (globalLights.lights[i].position.xyz - input.worldPos);
    light.range = globalLights.lights[i].range;
    light.color = globalLights.lights[i].color;
    light.intensity = globalLights.lights[i].intensity;
    let lightVis = pointLightVisibility(i, input.worldPos, light.pointToLight);
    Lo = (Lo + (lightRadiance(light, surface) * lightVis));
  }
  let ssaoCoord = (input.position.xy / vec2<f32>(textureDimensions(ssaoTexture).xy));
  let ssaoFactor = textureSample(ssaoTexture, defaultSampler, ssaoCoord).r;
  let ambient = (((globalLights.ambient * surface.albedo) * surface.ao) * ssaoFactor);
  let color = linearTosRGB(((Lo + ambient) + surface.emissive));
  var out : FragmentOutput;
  out.color = vec4(color, surface.baseColor.a);
  out.emissive = vec4(surface.emissive, surface.baseColor.a);
  return out;
}
