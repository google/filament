var<private> x_GLF_color : vec4<f32>;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  let x_20 : f32 = gl_FragCoord.x;
  let x_23 : f32 = gl_FragCoord.x;
  let x_26 : f32 = gl_FragCoord.y;
  let x_32 : f32 = gl_FragCoord.y;
  x_GLF_color = vec4<f32>((x_20 * 0.00390625), (f32((i32(x_23) ^ i32(x_26))) * 0.00390625), (x_32 * 0.00390625), 1.0);
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
