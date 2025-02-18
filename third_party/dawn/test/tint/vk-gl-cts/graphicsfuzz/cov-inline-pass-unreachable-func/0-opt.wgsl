var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn func_() -> f32 {
  var x : f32;
  x = 2.0;
  let x_35 : f32 = gl_FragCoord.x;
  if ((x_35 == 12.0)) {
    let x_40 : f32 = gl_FragCoord.y;
    if ((x_40 == 13.0)) {
      let x_44 : f32 = x;
      x = (x_44 + 1.0);
    }
    let x_46 : f32 = x;
    return x_46;
  }
  return 1.0;
}

fn main_1() {
  if (false) {
    let x_31 : f32 = func_();
    x_GLF_color = vec4<f32>(x_31, x_31, x_31, x_31);
  } else {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
