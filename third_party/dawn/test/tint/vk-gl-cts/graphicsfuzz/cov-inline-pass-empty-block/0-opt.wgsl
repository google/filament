var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn func_() -> vec4<f32> {
  var x : f32;
  x = 1.0;
  let x_30 : f32 = gl_FragCoord.x;
  if ((x_30 < 0.0)) {
    x = 0.5;
  }
  let x_34 : f32 = x;
  return vec4<f32>(x_34, 0.0, 0.0, 1.0);
}

fn main_1() {
  x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  loop {
    let x_26 : vec4<f32> = func_();
    x_GLF_color = x_26;
    if (false) {
    } else {
      break;
    }
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
