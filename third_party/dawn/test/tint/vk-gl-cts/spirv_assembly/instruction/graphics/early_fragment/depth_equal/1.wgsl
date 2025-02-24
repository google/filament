var<private> color_out : vec4<f32>;

var<private> gl_FragDepth : f32;

fn main_1() {
  color_out = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  gl_FragDepth = 0.300000012;
  return;
}

struct main_out {
  @location(0)
  color_out_1 : vec4<f32>,
  @builtin(frag_depth)
  gl_FragDepth_1 : f32,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(color_out, gl_FragDepth);
}
