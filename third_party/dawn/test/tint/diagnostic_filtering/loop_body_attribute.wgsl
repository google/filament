@fragment
fn main(@location(0) x : f32) {
  loop @diagnostic(warning, derivative_uniformity) {
    _ = dpdx(1.0);
    continuing {
      break if x > 0.0;
    }
  }
}
