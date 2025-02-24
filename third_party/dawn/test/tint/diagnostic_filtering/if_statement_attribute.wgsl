@group(0) @binding(1) var t : texture_2d<f32>;
@group(0) @binding(2) var s : sampler;

@fragment
fn main(@location(0) x : f32) {
  @diagnostic(warning, derivative_uniformity)
  if (x > 0) {
  } else if (dpdx(1.0) > 0)  {
  }
}
