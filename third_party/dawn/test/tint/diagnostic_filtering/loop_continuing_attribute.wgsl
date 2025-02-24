@fragment
fn main(@location(0) x : f32) {
  loop {
    continuing @diagnostic(warning, derivative_uniformity) {
      _ = dpdx(1.0);
      break if x > 0.0;
    }
  }
}
