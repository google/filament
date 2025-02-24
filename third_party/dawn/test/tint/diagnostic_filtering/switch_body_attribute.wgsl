@fragment
fn main(@location(0) x : f32) {
  switch (i32(x)) @diagnostic(warning, derivative_uniformity) {
    default {
      _ = dpdx(1.0);
    }
  }
}
