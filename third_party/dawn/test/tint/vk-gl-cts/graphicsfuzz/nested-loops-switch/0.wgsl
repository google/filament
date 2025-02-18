struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  var GLF_dead5cols : i32;
  var GLF_dead5rows : i32;
  var GLF_dead5c : i32;
  var GLF_dead5r : i32;
  var msb10 : i32;
  var donor_replacementGLF_dead5sums : array<f32, 9u>;
  i = 0;
  loop {
    let x_45 : i32 = i;
    let x_47 : f32 = x_6.injectionSwitch.x;
    if ((x_45 >= i32(x_47))) {
      break;
    }
    let x_53 : f32 = x_6.injectionSwitch.y;
    if ((0.0 > x_53)) {
      GLF_dead5cols = 2;
      loop {
        let x_61 : i32 = GLF_dead5cols;
        if ((x_61 <= 4)) {
        } else {
          break;
        }
        GLF_dead5rows = 2;
        loop {
          let x_68 : i32 = GLF_dead5rows;
          if ((x_68 <= 4)) {
          } else {
            break;
          }
          GLF_dead5c = 0;
          loop {
            let x_75 : i32 = GLF_dead5c;
            let x_76 : i32 = GLF_dead5cols;
            if ((x_75 < x_76)) {
            } else {
              break;
            }
            GLF_dead5r = 0;
            loop {
              let x_83 : i32 = GLF_dead5r;
              let x_84 : i32 = GLF_dead5rows;
              if ((x_83 < x_84)) {
              } else {
                break;
              }
              let x_87 : i32 = msb10;
              switch(x_87) {
                case 1, 8: {
                  let x_90 : i32 = msb10;
                  let x_92 : i32 = msb10;
                  let x_95 : i32 = msb10;
                  let x_96 : i32 = select(0, x_95, ((x_90 >= 0) & (x_92 < 9)));
                  let x_98 : f32 = donor_replacementGLF_dead5sums[x_96];
                  donor_replacementGLF_dead5sums[x_96] = (x_98 + 1.0);
                }
                default: {
                }
              }

              continuing {
                let x_101 : i32 = GLF_dead5r;
                GLF_dead5r = (x_101 + 1);
              }
            }

            continuing {
              let x_103 : i32 = GLF_dead5c;
              GLF_dead5c = (x_103 + 1);
            }
          }
          let x_105 : i32 = msb10;
          msb10 = (x_105 + 1);

          continuing {
            let x_107 : i32 = GLF_dead5rows;
            GLF_dead5rows = (x_107 + 1);
          }
        }

        continuing {
          let x_109 : i32 = GLF_dead5cols;
          GLF_dead5cols = (x_109 + 1);
        }
      }
    }
    let x_111 : i32 = i;
    i = (x_111 + 1);

    continuing {
      let x_113 : i32 = i;
      break if !(x_113 < 200);
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
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
