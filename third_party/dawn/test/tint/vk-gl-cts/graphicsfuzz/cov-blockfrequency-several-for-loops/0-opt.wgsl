var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var c : vec4<f32>;
  var a : i32;
  var i1 : i32;
  var i2 : i32;
  var i3 : i32;
  var i4 : i32;
  var i5 : i32;
  var i6 : i32;
  var i7 : i32;
  var i8_1 : i32;
  c = vec4<f32>(0.0, 0.0, 0.0, 1.0);
  a = 0;
  loop {
    loop {
      let x_46 : i32 = a;
      c[x_46] = 1.0;
      i1 = 0;
      loop {
        let x_52 : i32 = i1;
        if ((x_52 < 1)) {
        } else {
          break;
        }
        i2 = 0;
        loop {
          let x_59 : i32 = i2;
          if ((x_59 < 1)) {
          } else {
            break;
          }
          i3 = 0;
          loop {
            let x_66 : i32 = i3;
            if ((x_66 < 1)) {
            } else {
              break;
            }
            i4 = 0;
            loop {
              let x_73 : i32 = i4;
              if ((x_73 < 1)) {
              } else {
                break;
              }
              i5 = 0;
              loop {
                let x_80 : i32 = i5;
                if ((x_80 < 1)) {
                } else {
                  break;
                }
                i6 = 0;
                loop {
                  let x_87 : i32 = i6;
                  if ((x_87 < 1)) {
                  } else {
                    break;
                  }
                  i7 = 0;
                  loop {
                    let x_94 : i32 = i7;
                    if ((x_94 < 1)) {
                    } else {
                      break;
                    }
                    i8_1 = 0;
                    loop {
                      let x_101 : i32 = i8_1;
                      if ((x_101 < 17)) {
                      } else {
                        break;
                      }
                      let x_104 : i32 = a;
                      a = (x_104 + 1);

                      continuing {
                        let x_106 : i32 = i8_1;
                        i8_1 = (x_106 + 1);
                      }
                    }

                    continuing {
                      let x_108 : i32 = i7;
                      i7 = (x_108 + 1);
                    }
                  }

                  continuing {
                    let x_110 : i32 = i6;
                    i6 = (x_110 + 1);
                  }
                }

                continuing {
                  let x_112 : i32 = i5;
                  i5 = (x_112 + 1);
                }
              }

              continuing {
                let x_114 : i32 = i4;
                i4 = (x_114 + 1);
              }
            }

            continuing {
              let x_116 : i32 = i3;
              i3 = (x_116 + 1);
            }
          }

          continuing {
            let x_118 : i32 = i2;
            i2 = (x_118 + 1);
          }
        }

        continuing {
          let x_120 : i32 = i1;
          i1 = (x_120 + 1);
        }
      }

      continuing {
        let x_123 : f32 = gl_FragCoord.x;
        break if !(x_123 < -1.0);
      }
    }

    continuing {
      let x_126 : f32 = gl_FragCoord.y;
      break if !(x_126 < -1.0);
    }
  }
  let x_128 : vec4<f32> = c;
  x_GLF_color = x_128;
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
