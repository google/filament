<dawn>/test/tint/diagnostic_filtering/else_if_body_attribute.wgsl:8:9 warning: 'textureSample' must only be called from uniform control flow
    _ = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/else_if_body_attribute.wgsl:6:3 note: control flow depends on possibly non-uniform value
  if (x > 0) {
  ^^

<dawn>/test/tint/diagnostic_filtering/else_if_body_attribute.wgsl:6:7 note: user-defined input 'x' of 'main' may be non-uniform
  if (x > 0) {
      ^

@group(0) @binding(1) var t : texture_2d<f32>;

@group(0) @binding(2) var s : sampler;

@fragment
fn main(@location(0) x : f32) {
  if ((x > 0)) {
  } else if ((x < 0)) @diagnostic(warning, derivative_uniformity) {
    _ = textureSample(t, s, vec2(0, 0));
  }
}
