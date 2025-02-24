<dawn>/test/tint/diagnostic_filtering/loop_attribute.wgsl:5:9 warning: 'dpdx' must only be called from uniform control flow
    _ = dpdx(1.0);
        ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/loop_attribute.wgsl:7:7 note: control flow depends on possibly non-uniform value
      break if x > 0.0;
      ^^^^^

<dawn>/test/tint/diagnostic_filtering/loop_attribute.wgsl:7:16 note: user-defined input 'x' of 'main' may be non-uniform
      break if x > 0.0;
               ^

@fragment
fn main(@location(0) x : f32) {
  @diagnostic(warning, derivative_uniformity) loop {
    _ = dpdx(1.0);

    continuing {
      break if (x > 0.0);
    }
  }
}
