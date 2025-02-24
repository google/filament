struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var loop_count : i32;
  loop_count = 0;
  let x_33 : f32 = x_7.injectionSwitch.x;
  let x_35 : f32 = x_7.injectionSwitch.y;
  let x_36 : bool = (x_33 > x_35);
  if (x_36) {
    return;
  }
  let x_40 : f32 = gl_FragCoord.x;
  let x_41 : bool = (x_40 < 0.0);
  loop {
    let x_43 : i32 = loop_count;
    if ((x_43 < 100)) {
    } else {
      break;
    }
    if (x_36) {
      break;
    }
    if (x_36) {
      x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
    } else {
      if (x_41) {
        return;
      }
    }
    if (x_36) {
      x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
    } else {
      x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    }
    if (x_36) {
      return;
    }
    if (x_41) {
      loop {
        let x_63 : i32 = loop_count;
        if ((x_63 < 100)) {
        } else {
          break;
        }

        continuing {
          let x_67 : i32 = loop_count;
          loop_count = (x_67 + 1);
        }
      }
    }

    continuing {
      let x_69 : i32 = loop_count;
      loop_count = (x_69 + 1);
    }
  }
  let x_71 : i32 = loop_count;
  if ((x_71 >= 100)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
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
