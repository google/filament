var<private> x_GLF_color : vec4<f32>;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  var i : i32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  i = 1;
  loop {
    let x_6 : i32 = i;
    if ((x_6 < 2)) {
    } else {
      break;
    }
    let x_37 : f32 = gl_FragCoord.y;
    if ((x_37 < 0.0)) {
      let x_42 : f32 = gl_FragCoord.x;
      if ((x_42 < 0.0)) {
        x_GLF_color = vec4<f32>(226.695999146, 1.0, 1.0, 1.0);
      }
      continue;
    }
    return;

    continuing {
      let x_7 : i32 = i;
      i = (x_7 + 1);
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
