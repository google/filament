var<private> x_GLF_color : vec4<f32>;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  var x_30 : f32;
  var foo : u32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  let x_32 : f32 = gl_FragCoord.x;
  if ((x_32 > -1.0)) {
    let x_38 : f32 = x_GLF_color.x;
    x_30 = x_38;
  } else {
    let x_6 : u32 = foo;
    let x_7 : u32 = (x_6 - bitcast<u32>(1));
    foo = x_7;
    x_30 = f32((178493u + x_7));
  }
  let x_40 : f32 = x_30;
  x_GLF_color.x = x_40;
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
