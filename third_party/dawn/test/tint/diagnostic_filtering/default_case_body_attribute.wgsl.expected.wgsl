<dawn>/test/tint/diagnostic_filtering/default_case_body_attribute.wgsl:8:11 warning: 'textureSample' must only be called from uniform control flow
      _ = textureSample(t, s, vec2(0, 0));
          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/default_case_body_attribute.wgsl:6:3 note: control flow depends on possibly non-uniform value
  switch (i32(x)) {
  ^^^^^^

<dawn>/test/tint/diagnostic_filtering/default_case_body_attribute.wgsl:6:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x)) {
              ^

@group(0) @binding(1) var t : texture_2d<f32>;

@group(0) @binding(2) var s : sampler;

@fragment
fn main(@location(0) x : f32) {
  switch(i32(x)) {
    default: @diagnostic(warning, derivative_uniformity) {
      _ = textureSample(t, s, vec2(0, 0));
    }
  }
}
