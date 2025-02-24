struct buf0 {
  two : f32,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : f32;
  var b : f32;
  let x_33 : f32 = gl_FragCoord.x;
  a = dpdx(cos(x_33));
  let x_37 : f32 = x_8.two;
  let x_38 : f32 = a;
  b = mix(2.0, x_37, x_38);
  let x_40 : f32 = b;
  let x_42 : f32 = b;
  if (((x_40 >= 1.899999976) & (x_42 <= 2.099999905))) {
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
