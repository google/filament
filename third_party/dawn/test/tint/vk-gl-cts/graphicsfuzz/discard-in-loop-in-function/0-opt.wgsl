struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn f_() {
  loop {
    let x_35 : f32 = x_7.injectionSwitch.y;
    if ((1.0 > x_35)) {
      let x_40 : f32 = gl_FragCoord.y;
      if ((x_40 < 0.0)) {
        continue;
      } else {
        continue;
      }
    }
    discard;

    continuing {
      break if !(false);
    }
  }
  return;
}

fn main_1() {
  f_();
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
