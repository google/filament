struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> gv : f32;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var lv : f32;
  var x_43 : f32;
  var GLF_live5r : i32;
  var GLF_live5_looplimiter6 : i32;
  let x_45 : f32 = x_7.injectionSwitch.y;
  if ((1.0 > x_45)) {
    let x_50 : f32 = gv;
    x_43 = abs(x_50);
  } else {
    x_43 = 260.0;
  }
  let x_52 : f32 = x_43;
  lv = x_52;
  let x_53 : f32 = lv;
  if ((i32(x_53) < 250)) {
    let x_58 : f32 = lv;
    if ((i32(x_58) < 180)) {
      let x_64 : f32 = lv;
      let x_65 : f32 = clamp(x_64, 1.0, 1.0);
    } else {
      let x_67 : f32 = gl_FragCoord.y;
      if ((x_67 < 0.0)) {
        let x_71 : f32 = lv;
        if ((i32(x_71) < 210)) {
          loop {

            continuing {
              break if !(true);
            }
          }
        }
        GLF_live5r = 0;
        loop {
          if (true) {
          } else {
            break;
          }
          let x_11 : i32 = GLF_live5_looplimiter6;
          if ((x_11 >= 6)) {
            break;
          }
          let x_12 : i32 = GLF_live5_looplimiter6;
          GLF_live5_looplimiter6 = (x_12 + 1);
        }
      }
    }
  }
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
