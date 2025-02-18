struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn computePoint_() -> vec3<f32> {
  let x_48 : f32 = x_7.injectionSwitch.x;
  let x_50 : f32 = x_7.injectionSwitch.y;
  if ((x_48 > x_50)) {
    discard;
  }
  return vec3<f32>();
}

fn main_1() {
  var x_34 : bool = false;
  loop {
    let x_36 : vec3<f32> = computePoint_();
    let x_41 : f32 = gl_FragCoord.x;
    if ((x_41 < 0.0)) {
      x_34 = true;
      break;
    }
    let x_45 : vec3<f32> = computePoint_();
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    x_34 = true;
    break;
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
