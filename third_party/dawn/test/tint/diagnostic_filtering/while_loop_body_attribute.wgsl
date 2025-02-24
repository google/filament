@group(0) @binding(1) var t : texture_2d<f32>;
@group(0) @binding(2) var s : sampler;

@fragment
fn main(@location(0) x : f32) {
  var v = vec4<f32>(0);
  while (x > v.x) @diagnostic(warning, derivative_uniformity) {
    v = textureSample(t, s, vec2(0, 0));
  }
}
