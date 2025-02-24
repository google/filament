<dawn>/test/tint/diagnostic_filtering/for_loop_attribute.wgsl:5:21 warning: 'dpdx' must only be called from uniform control flow
  for (; x > v.x && dpdx(1.0) > 0.0; ) {
                    ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/for_loop_attribute.wgsl:5:3 note: control flow depends on possibly non-uniform value
  for (; x > v.x && dpdx(1.0) > 0.0; ) {
  ^^^

<dawn>/test/tint/diagnostic_filtering/for_loop_attribute.wgsl:5:21 note: return value of 'dpdx' may be non-uniform
  for (; x > v.x && dpdx(1.0) > 0.0; ) {
                    ^^^^^^^^^

@fragment
fn main(@location(0) x : f32) {
  var v = vec4<f32>(0);
  @diagnostic(warning, derivative_uniformity) for(; ((x > v.x) && (dpdx(1.0) > 0.0)); ) {
  }
}
