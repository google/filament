struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> x_GLF_color : vec4<f32>;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_8 : buf0;

fn main_1() {
  var odd_index : i32;
  var even_index : i32;
  var j : i32;
  var ll : i32;
  var x_59 : bool;
  var x_60_phi : bool;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  let x_53 : f32 = gl_FragCoord.x;
  let x_54 : bool = (x_53 < 128.0);
  x_60_phi = x_54;
  if (x_54) {
    let x_58 : f32 = gl_FragCoord.y;
    x_59 = (x_58 < 128.0);
    x_60_phi = x_59;
  }
  let x_60 : bool = x_60_phi;
  if (x_60) {
    return;
  }
  odd_index = 0;
  loop {
    let x_11 : i32 = odd_index;
    if ((x_11 <= 1)) {
    } else {
      break;
    }
    let x_70 : f32 = x_GLF_color.x;
    x_GLF_color.x = (x_70 + 0.25);
    let x_12 : i32 = odd_index;
    odd_index = (x_12 + 1);
  }
  even_index = 1;
  loop {
    let x_14 : i32 = even_index;
    if ((x_14 >= 0)) {
    } else {
      break;
    }
    let x_80 : f32 = x_GLF_color.x;
    x_GLF_color.x = (x_80 + 0.25);
    let x_84 : f32 = x_8.injectionSwitch.x;
    let x_86 : f32 = x_8.injectionSwitch.y;
    if ((x_84 > x_86)) {
      continue;
    }
    let x_15 : i32 = even_index;
    if ((x_15 >= 1)) {
      discard;
    }
    j = 1;
    loop {
      if (true) {
      } else {
        break;
      }
      let x_16 : i32 = ll;
      if ((x_16 >= 3)) {
        break;
      }
      let x_17 : i32 = ll;
      ll = (x_17 + 1);
      let x_19 : i32 = j;
      if ((bitcast<u32>(x_19) < 1u)) {
        continue;
      }
      let x_106 : f32 = x_8.injectionSwitch.x;
      let x_108 : f32 = x_8.injectionSwitch.y;
      if ((x_106 > x_108)) {
        break;
      }

      continuing {
        let x_20 : i32 = j;
        j = (x_20 + 1);
      }
    }
    let x_113 : f32 = x_8.injectionSwitch.x;
    let x_115 : f32 = x_8.injectionSwitch.y;
    if ((x_113 > x_115)) {
      x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
    }
    let x_22 : i32 = even_index;
    even_index = (x_22 - 1);
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
