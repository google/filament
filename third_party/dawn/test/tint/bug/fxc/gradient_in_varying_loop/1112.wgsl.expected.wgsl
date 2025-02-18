@group(0) @binding(0) var Sampler : sampler;

@group(0) @binding(1) var randomTexture : texture_2d<f32>;

@group(0) @binding(2) var depthTexture : texture_2d<f32>;

@fragment
fn main(@location(0) vUV : vec2<f32>) -> @location(0) vec4<f32> {
  let random : vec3<f32> = textureSample(randomTexture, Sampler, vUV).rgb;
  var i = 0;
  loop {
    if ((i < 1)) {
    } else {
      break;
    }
    let offset : vec3<f32> = vec3<f32>(random.x);
    if (((((offset.x < 0.0) || (offset.y < 0.0)) || (offset.x > 1.0)) || (offset.y > 1.0))) {
      i = (i + 1);
      continue;
    }
    let sampleDepth : f32 = 0;
    i = (i + 1);
  }
  return vec4<f32>(1.0);
}
