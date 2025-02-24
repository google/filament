@fragment
fn main(@location(0) x : f32) {
  @diagnostic(warning, derivative_uniformity)
  loop {
    _ = dpdx(1.0);
    continuing {
      break if x > 0.0;
    }
  }
}
