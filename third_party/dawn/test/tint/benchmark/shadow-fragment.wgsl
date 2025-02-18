const shadowDepthTextureSize : f32 = 1024.0;

struct Scene {
  lightViewProjMatrix : mat4x4<f32>,
  cameraViewProjMatrix : mat4x4<f32>,
  lightPos : vec3<f32>,
}

@group(0) @binding(0) var<uniform> scene : Scene;

@group(0) @binding(1) var shadowMap : texture_depth_2d;

@group(0) @binding(2) var shadowSampler : sampler_comparison;

struct FragmentInput {
  @location(0)
  shadowPos : vec3<f32>,
  @location(1)
  fragPos : vec3<f32>,
  @location(2)
  fragNorm : vec3<f32>,
}

const albedo : vec3<f32> = vec3<f32>(0.899999976, 0.899999976, 0.899999976);

const ambientFactor : f32 = 0.200000003;

@fragment
fn main(input : FragmentInput) -> @location(0) vec4<f32> {
  var visibility : f32 = 0.0;
  let oneOverShadowDepthTextureSize = (1.0 / shadowDepthTextureSize);
  for(var y : i32 = -1; (y <= 1); y = (y + 1)) {
    for(var x : i32 = -1; (x <= 1); x = (x + 1)) {
      let offset : vec2<f32> = vec2<f32>((f32(x) * oneOverShadowDepthTextureSize), (f32(y) * oneOverShadowDepthTextureSize));
      visibility = (visibility + textureSampleCompare(shadowMap, shadowSampler, (input.shadowPos.xy + offset), (input.shadowPos.z - 0.007)));
    }
  }
  visibility = (visibility / 9.0);
  let lambertFactor : f32 = max(dot(normalize((scene.lightPos - input.fragPos)), input.fragNorm), 0.0);
  let lightingFactor : f32 = min((ambientFactor + (visibility * lambertFactor)), 1.0);
  return vec4<f32>((lightingFactor * albedo), 1.0);
}
