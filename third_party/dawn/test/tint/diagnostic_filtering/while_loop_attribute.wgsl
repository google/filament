@fragment
fn main(@location(0) x : f32) {
  var v = vec4<f32>(0);
  @diagnostic(warning, derivative_uniformity)
  while (x > 0.0 && dpdx(1.0) > 0.0)  {
  }
}
