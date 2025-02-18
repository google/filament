<dawn>/test/tint/diagnostic_filtering/if_statement_attribute.wgsl:8:14 warning: 'dpdx' must only be called from uniform control flow
  } else if (dpdx(1.0) > 0)  {
             ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/if_statement_attribute.wgsl:7:3 note: control flow depends on possibly non-uniform value
  if (x > 0) {
  ^^

<dawn>/test/tint/diagnostic_filtering/if_statement_attribute.wgsl:7:7 note: user-defined input 'x' of 'main' may be non-uniform
  if (x > 0) {
      ^

@group(0) @binding(1) var t : texture_2d<f32>;

@group(0) @binding(2) var s : sampler;

@fragment
fn main(@location(0) x : f32) {
  @diagnostic(warning, derivative_uniformity) if ((x > 0)) {
  } else if ((dpdx(1.0) > 0)) {
  }
}
