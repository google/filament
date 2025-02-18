struct FragmentInput {
  @location(2)
  vUv : vec2<f32>,
}

struct FragmentOutput {
  @location(0)
  color : vec4<f32>,
}

@binding(5) @group(1) var depthMap : texture_depth_2d;

@binding(3) @group(1) var texSampler : sampler;

@fragment
fn main(fIn : FragmentInput) -> FragmentOutput {
  let sample : f32 = textureSample(depthMap, texSampler, fIn.vUv);
  let color : vec3<f32> = vec3<f32>(sample, sample, sample);
  var fOut : FragmentOutput;
  fOut.color = vec4<f32>(color, 1.0);
  return fOut;
}
