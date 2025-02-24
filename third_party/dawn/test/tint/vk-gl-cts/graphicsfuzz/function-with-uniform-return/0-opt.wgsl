struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn fx_() -> f32 {
  let x_50 : f32 = gl_FragCoord.y;
  if ((x_50 >= 0.0)) {
    let x_55 : f32 = x_7.injectionSwitch.y;
    return x_55;
  }
  loop {
    if (true) {
    } else {
      break;
    }
    x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  }
  return 0.0;
}

fn main_1() {
  var x2 : f32;
  var B : f32;
  var k0 : f32;
  x2 = 1.0;
  B = 1.0;
  let x_34 : f32 = fx_();
  x_GLF_color = vec4<f32>(x_34, 0.0, 0.0, 1.0);
  loop {
    let x_40 : f32 = x2;
    if ((x_40 > 2.0)) {
    } else {
      break;
    }
    let x_43 : f32 = fx_();
    let x_44 : f32 = fx_();
    k0 = (x_43 - x_44);
    let x_46 : f32 = k0;
    B = x_46;
    let x_47 : f32 = B;
    x2 = x_47;
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
