struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  var i : i32;
  var i_1 : i32;
  var i_2 : i32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  i = 0;
  let x_35 : f32 = x_7.injectionSwitch.y;
  if ((x_35 < 0.0)) {
  } else {
    var x_42 : bool;
    let x_41 : f32 = gl_FragCoord.y;
    x_42 = (x_41 < -1.0);
    if (x_42) {
    } else {
      loop {
        let x_50 : i32 = i;
        if ((x_50 >= 256)) {
          break;
        }
        loop {
          i_1 = 0;
          loop {
            let x_58 : i32 = i_1;
            if ((x_58 < 1)) {
            } else {
              break;
            }
            if (x_42) {
              i_2 = 0;
              loop {
                let x_66 : i32 = i_2;
                if ((x_66 < 1)) {
                } else {
                  break;
                }

                continuing {
                  let x_70 : i32 = i_2;
                  i_2 = (x_70 + 1);
                }
              }
              continue;
            }
            return;

            continuing {
              let x_72 : i32 = i_1;
              i_1 = (x_72 + 1);
            }
          }

          continuing {
            break if !(false);
          }
        }

        continuing {
          break if !(false);
        }
      }
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
