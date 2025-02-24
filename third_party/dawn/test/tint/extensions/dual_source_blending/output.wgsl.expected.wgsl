enable dual_source_blending;

struct FragOutput {
  @location(0) @blend_src(0)
  color : vec4<f32>,
  @location(0) @blend_src(1)
  blend : vec4<f32>,
}

@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  output.blend = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}
