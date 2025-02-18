struct buf0 {
  three : i32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_36 : bool;
  var x_37_phi : bool;
  let x_29 : i32 = x_6.three;
  let x_30 : bool = (x_29 > 1);
  x_37_phi = x_30;
  if (x_30) {
    let x_34 : f32 = gl_FragCoord.y;
    x_36 = !((x_34 < -5.0));
    x_37_phi = x_36;
  }
  let x_37 : bool = x_37_phi;
  if (x_37) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  }
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
