var<private> outColor : vec4<f32>;

var<private> gl_FragDepth : f32;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  outColor = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  let x_20 : f32 = gl_FragCoord.z;
  gl_FragDepth = x_20;
  return;
}

struct main_out {
  @location(0)
  outColor_1 : vec4<f32>,
  @builtin(frag_depth)
  gl_FragDepth_1 : f32,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(outColor, gl_FragDepth);
}
