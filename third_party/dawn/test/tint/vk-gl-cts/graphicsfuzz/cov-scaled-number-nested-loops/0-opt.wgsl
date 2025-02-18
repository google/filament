struct buf0 {
  one : i32,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var i0 : i32;
  var i1 : i32;
  var i2 : i32;
  var i3 : i32;
  var i4 : i32;
  var i5 : i32;
  var i6 : i32;
  var i7 : i32;
  var i8_1 : i32;
  var i9 : i32;
  a = 0;
  i0 = 0;
  loop {
    let x_40 : i32 = i0;
    let x_42 : i32 = x_7.one;
    if ((x_40 < x_42)) {
    } else {
      break;
    }
    i1 = 0;
    loop {
      let x_49 : i32 = i1;
      let x_51 : i32 = x_7.one;
      if ((x_49 < x_51)) {
      } else {
        break;
      }
      i2 = 0;
      loop {
        let x_58 : i32 = i2;
        let x_60 : i32 = x_7.one;
        if ((x_58 < x_60)) {
        } else {
          break;
        }
        i3 = 0;
        loop {
          let x_67 : i32 = i3;
          let x_69 : i32 = x_7.one;
          if ((x_67 < (x_69 + 2))) {
          } else {
            break;
          }
          i4 = 0;
          loop {
            let x_77 : i32 = i4;
            let x_79 : i32 = x_7.one;
            if ((x_77 < x_79)) {
            } else {
              break;
            }
            i5 = 0;
            loop {
              let x_86 : i32 = i5;
              let x_88 : i32 = x_7.one;
              if ((x_86 < x_88)) {
              } else {
                break;
              }
              loop {
                let x_96 : i32 = x_7.one;
                if ((x_96 > 0)) {
                } else {
                  break;
                }
                i6 = 0;
                loop {
                  let x_103 : i32 = i6;
                  let x_105 : i32 = x_7.one;
                  if ((x_103 < x_105)) {
                  } else {
                    break;
                  }
                  i7 = 0;
                  loop {
                    let x_112 : i32 = i7;
                    let x_114 : i32 = x_7.one;
                    if ((x_112 < x_114)) {
                    } else {
                      break;
                    }
                    i8_1 = 0;
                    loop {
                      let x_121 : i32 = i8_1;
                      let x_123 : i32 = x_7.one;
                      if ((x_121 < x_123)) {
                      } else {
                        break;
                      }
                      i9 = 0;
                      loop {
                        let x_130 : i32 = i9;
                        let x_132 : i32 = x_7.one;
                        if ((x_130 < x_132)) {
                        } else {
                          break;
                        }
                        let x_135 : i32 = a;
                        a = (x_135 + 1);

                        continuing {
                          let x_137 : i32 = i9;
                          i9 = (x_137 + 1);
                        }
                      }

                      continuing {
                        let x_139 : i32 = i8_1;
                        i8_1 = (x_139 + 1);
                      }
                    }

                    continuing {
                      let x_141 : i32 = i7;
                      i7 = (x_141 + 1);
                    }
                  }

                  continuing {
                    let x_143 : i32 = i6;
                    i6 = (x_143 + 1);
                  }
                }
                break;
              }

              continuing {
                let x_145 : i32 = i5;
                i5 = (x_145 + 1);
              }
            }

            continuing {
              let x_147 : i32 = i4;
              i4 = (x_147 + 1);
            }
          }

          continuing {
            let x_149 : i32 = i3;
            i3 = (x_149 + 1);
          }
        }

        continuing {
          let x_151 : i32 = i2;
          i2 = (x_151 + 1);
        }
      }

      continuing {
        let x_153 : i32 = i1;
        i1 = (x_153 + 1);
      }
    }

    continuing {
      let x_155 : i32 = i0;
      i0 = (x_155 + 1);
    }
  }
  let x_157 : i32 = a;
  if ((x_157 == 3)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  }
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
