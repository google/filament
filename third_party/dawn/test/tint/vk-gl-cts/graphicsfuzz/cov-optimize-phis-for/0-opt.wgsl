struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 3u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

@group(0) @binding(1) var<uniform> x_11 : buf1;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : f32;
  var b : f32;
  var i : i32;
  var i_1 : i32;
  var i_2 : i32;
  var i_3 : i32;
  var i_4 : i32;
  var i_5 : i32;
  var i_6 : i32;
  var i_7 : i32;
  var i_8 : i32;
  var i_9 : i32;
  var i_10 : i32;
  let x_104 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  a = x_104;
  let x_106 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  b = x_106;
  let x_24 : i32 = x_11.x_GLF_uniform_int_values[1].el;
  i = x_24;
  loop {
    let x_25 : i32 = i;
    let x_26 : i32 = x_11.x_GLF_uniform_int_values[0].el;
    if ((x_25 < x_26)) {
    } else {
      break;
    }
    let x_27 : i32 = x_11.x_GLF_uniform_int_values[1].el;
    i_1 = x_27;
    loop {
      let x_28 : i32 = i_1;
      let x_29 : i32 = x_11.x_GLF_uniform_int_values[0].el;
      if ((x_28 < x_29)) {
      } else {
        break;
      }
      let x_30 : i32 = x_11.x_GLF_uniform_int_values[1].el;
      i_2 = x_30;
      loop {
        let x_31 : i32 = i_2;
        let x_32 : i32 = x_11.x_GLF_uniform_int_values[0].el;
        if ((x_31 < x_32)) {
        } else {
          break;
        }
        let x_33 : i32 = x_11.x_GLF_uniform_int_values[2].el;
        i_3 = x_33;
        loop {
          let x_34 : i32 = i_3;
          let x_35 : i32 = x_11.x_GLF_uniform_int_values[0].el;
          if ((x_34 < x_35)) {
          } else {
            break;
          }
          let x_36 : i32 = x_11.x_GLF_uniform_int_values[2].el;
          i_4 = x_36;
          loop {
            let x_37 : i32 = i_4;
            let x_38 : i32 = x_11.x_GLF_uniform_int_values[0].el;
            if ((x_37 < x_38)) {
            } else {
              break;
            }
            let x_39 : i32 = x_11.x_GLF_uniform_int_values[1].el;
            i_5 = x_39;
            loop {
              let x_40 : i32 = i_5;
              let x_41 : i32 = x_11.x_GLF_uniform_int_values[0].el;
              if ((x_40 < x_41)) {
              } else {
                break;
              }
              let x_42 : i32 = x_11.x_GLF_uniform_int_values[1].el;
              i_6 = x_42;
              loop {
                let x_43 : i32 = i_6;
                let x_44 : i32 = x_11.x_GLF_uniform_int_values[0].el;
                if ((x_43 < x_44)) {
                } else {
                  break;
                }
                let x_45 : i32 = x_11.x_GLF_uniform_int_values[1].el;
                i_7 = x_45;
                loop {
                  let x_46 : i32 = i_7;
                  let x_47 : i32 = x_11.x_GLF_uniform_int_values[0].el;
                  if ((x_46 < x_47)) {
                  } else {
                    break;
                  }
                  let x_48 : i32 = x_11.x_GLF_uniform_int_values[1].el;
                  i_8 = x_48;
                  loop {
                    let x_49 : i32 = i_8;
                    let x_50 : i32 = x_11.x_GLF_uniform_int_values[0].el;
                    if ((x_49 < x_50)) {
                    } else {
                      break;
                    }
                    let x_51 : i32 = x_11.x_GLF_uniform_int_values[1].el;
                    i_9 = x_51;
                    loop {
                      let x_52 : i32 = i_9;
                      let x_53 : i32 = x_11.x_GLF_uniform_int_values[0].el;
                      if ((x_52 < x_53)) {
                      } else {
                        break;
                      }
                      let x_54 : i32 = x_11.x_GLF_uniform_int_values[1].el;
                      i_10 = x_54;
                      loop {
                        let x_55 : i32 = i_10;
                        let x_56 : i32 = x_11.x_GLF_uniform_int_values[0].el;
                        if ((x_55 < x_56)) {
                        } else {
                          break;
                        }
                        let x_196 : f32 = x_7.x_GLF_uniform_float_values[1].el;
                        a = x_196;
                        let x_198 : f32 = gl_FragCoord.y;
                        let x_200 : f32 = x_7.x_GLF_uniform_float_values[1].el;
                        if ((x_198 > x_200)) {
                          break;
                        }

                        continuing {
                          let x_57 : i32 = i_10;
                          i_10 = (x_57 + 1);
                        }
                      }

                      continuing {
                        let x_59 : i32 = i_9;
                        i_9 = (x_59 + 1);
                      }
                    }

                    continuing {
                      let x_61 : i32 = i_8;
                      i_8 = (x_61 + 1);
                    }
                  }

                  continuing {
                    let x_63 : i32 = i_7;
                    i_7 = (x_63 + 1);
                  }
                }

                continuing {
                  let x_65 : i32 = i_6;
                  i_6 = (x_65 + 1);
                }
              }

              continuing {
                let x_67 : i32 = i_5;
                i_5 = (x_67 + 1);
              }
            }

            continuing {
              let x_69 : i32 = i_4;
              i_4 = (x_69 + 1);
            }
          }

          continuing {
            let x_71 : i32 = i_3;
            i_3 = (x_71 + 1);
          }
        }

        continuing {
          let x_73 : i32 = i_2;
          i_2 = (x_73 + 1);
        }
      }

      continuing {
        let x_75 : i32 = i_1;
        i_1 = (x_75 + 1);
      }
    }
    let x_204 : f32 = b;
    b = (x_204 + 1.0);

    continuing {
      let x_77 : i32 = i;
      i = (x_77 + 1);
    }
  }
  let x_206 : f32 = b;
  let x_207 : f32 = a;
  let x_208 : f32 = a;
  let x_209 : f32 = b;
  x_GLF_color = vec4<f32>(x_206, x_207, x_208, x_209);
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
