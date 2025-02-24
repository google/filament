var<private> result : vec4<f32>;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  let x_19 : f32 = gl_FragCoord.x;
  let x_23 : f32 = gl_FragCoord.y;
  result = vec4<f32>((floor(x_19) / 255.0), (floor(x_23) / 255.0), 0.0, 0.0);
  return;
}

struct main_out {
  @location(0)
  result_1 : vec4<f32>,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(result);
}
