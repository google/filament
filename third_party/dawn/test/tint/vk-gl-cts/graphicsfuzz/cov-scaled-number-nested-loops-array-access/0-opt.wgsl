struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 7u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 3u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(1) var<uniform> x_8 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var sums : array<f32, 2u>;
  var a : i32;
  var b : i32;
  var c : i32;
  var d : i32;
  var e : i32;
  var f : i32;
  var g : i32;
  var h : i32;
  var i : i32;
  var j : i32;
  var x_215 : bool;
  var x_216_phi : bool;
  let x_20 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_110 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  sums[x_20] = x_110;
  let x_22 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_114 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  sums[x_22] = x_114;
  let x_23 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  a = x_23;
  loop {
    let x_24 : i32 = a;
    let x_25 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_24 < x_25)) {
    } else {
      break;
    }
    let x_26 : i32 = x_6.x_GLF_uniform_int_values[5].el;
    b = x_26;
    loop {
      let x_27 : i32 = b;
      let x_28 : i32 = x_6.x_GLF_uniform_int_values[3].el;
      if ((x_27 < x_28)) {
      } else {
        break;
      }
      let x_29 : i32 = x_6.x_GLF_uniform_int_values[6].el;
      c = x_29;
      loop {
        let x_30 : i32 = c;
        let x_31 : i32 = x_6.x_GLF_uniform_int_values[4].el;
        if ((x_30 <= x_31)) {
        } else {
          break;
        }
        let x_32 : i32 = x_6.x_GLF_uniform_int_values[1].el;
        d = x_32;
        loop {
          let x_33 : i32 = d;
          let x_34 : i32 = x_6.x_GLF_uniform_int_values[6].el;
          if ((x_33 < x_34)) {
          } else {
            break;
          }
          let x_35 : i32 = x_6.x_GLF_uniform_int_values[0].el;
          e = x_35;
          loop {
            let x_36 : i32 = e;
            let x_37 : i32 = x_6.x_GLF_uniform_int_values[4].el;
            if ((x_36 <= x_37)) {
            } else {
              break;
            }
            let x_38 : i32 = x_6.x_GLF_uniform_int_values[1].el;
            f = x_38;
            loop {
              let x_39 : i32 = f;
              let x_40 : i32 = x_6.x_GLF_uniform_int_values[0].el;
              if ((x_39 < x_40)) {
              } else {
                break;
              }
              let x_41 : i32 = x_6.x_GLF_uniform_int_values[1].el;
              g = x_41;
              loop {
                let x_42 : i32 = g;
                let x_43 : i32 = x_6.x_GLF_uniform_int_values[6].el;
                if ((x_42 < x_43)) {
                } else {
                  break;
                }
                let x_44 : i32 = x_6.x_GLF_uniform_int_values[1].el;
                h = x_44;
                loop {
                  let x_45 : i32 = h;
                  let x_46 : i32 = x_6.x_GLF_uniform_int_values[0].el;
                  if ((x_45 < x_46)) {
                  } else {
                    break;
                  }
                  let x_47 : i32 = x_6.x_GLF_uniform_int_values[1].el;
                  i = x_47;
                  loop {
                    let x_48 : i32 = i;
                    let x_49 : i32 = x_6.x_GLF_uniform_int_values[4].el;
                    if ((x_48 < x_49)) {
                    } else {
                      break;
                    }
                    let x_50 : i32 = x_6.x_GLF_uniform_int_values[0].el;
                    j = x_50;
                    loop {
                      let x_51 : i32 = j;
                      let x_52 : i32 = x_6.x_GLF_uniform_int_values[1].el;
                      if ((x_51 > x_52)) {
                      } else {
                        break;
                      }
                      let x_53 : i32 = a;
                      let x_197 : f32 = x_8.x_GLF_uniform_float_values[2].el;
                      let x_199 : f32 = sums[x_53];
                      sums[x_53] = (x_199 + x_197);

                      continuing {
                        let x_54 : i32 = j;
                        j = (x_54 - 1);
                      }
                    }

                    continuing {
                      let x_56 : i32 = i;
                      i = (x_56 + 1);
                    }
                  }

                  continuing {
                    let x_58 : i32 = h;
                    h = (x_58 + 1);
                  }
                }

                continuing {
                  let x_60 : i32 = g;
                  g = (x_60 + 1);
                }
              }

              continuing {
                let x_62 : i32 = f;
                f = (x_62 + 1);
              }
            }

            continuing {
              let x_64 : i32 = e;
              e = (x_64 + 1);
            }
          }

          continuing {
            let x_66 : i32 = d;
            d = (x_66 + 1);
          }
        }

        continuing {
          let x_68 : i32 = c;
          c = (x_68 + 1);
        }
      }

      continuing {
        let x_70 : i32 = b;
        b = (x_70 + 1);
      }
    }

    continuing {
      let x_72 : i32 = a;
      a = (x_72 + 1);
    }
  }
  let x_74 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_204 : f32 = sums[x_74];
  let x_206 : f32 = x_8.x_GLF_uniform_float_values[1].el;
  let x_207 : bool = (x_204 == x_206);
  x_216_phi = x_207;
  if (x_207) {
    let x_75 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_212 : f32 = sums[x_75];
    let x_214 : f32 = x_8.x_GLF_uniform_float_values[1].el;
    x_215 = (x_212 == x_214);
    x_216_phi = x_215;
  }
  let x_216 : bool = x_216_phi;
  if (x_216) {
    let x_76 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_77 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_78 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_79 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_76), f32(x_77), f32(x_78), f32(x_79));
  } else {
    let x_80 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_230 : f32 = f32(x_80);
    x_GLF_color = vec4<f32>(x_230, x_230, x_230, x_230);
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
