struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 4u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

var<private> x_GLF_global_loop_count : i32;

@group(0) @binding(0) var<uniform> x_7 : buf0;

@group(0) @binding(1) var<uniform> x_10 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
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
  var i_11 : i32;
  var i_12 : i32;
  var i_13 : i32;
  var i_14 : i32;
  var sum : f32;
  var r : i32;
  x_GLF_global_loop_count = 0;
  let x_53 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  f = x_53;
  let x_55 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  i = x_55;
  loop {
    let x_60 : i32 = i;
    let x_62 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    if ((x_60 < x_62)) {
    } else {
      break;
    }
    let x_66 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    i_1 = x_66;
    loop {
      let x_71 : i32 = i_1;
      let x_73 : i32 = x_10.x_GLF_uniform_int_values[0].el;
      if ((x_71 < x_73)) {
      } else {
        break;
      }
      let x_77 : i32 = x_10.x_GLF_uniform_int_values[1].el;
      i_2 = x_77;
      loop {
        let x_82 : i32 = i_2;
        let x_84 : i32 = x_10.x_GLF_uniform_int_values[0].el;
        if ((x_82 < x_84)) {
        } else {
          break;
        }
        let x_88 : i32 = x_10.x_GLF_uniform_int_values[1].el;
        i_3 = x_88;
        loop {
          let x_93 : i32 = i_3;
          let x_95 : i32 = x_10.x_GLF_uniform_int_values[0].el;
          if ((x_93 < x_95)) {
          } else {
            break;
          }
          let x_99 : i32 = x_10.x_GLF_uniform_int_values[1].el;
          i_4 = x_99;
          loop {
            let x_104 : i32 = i_4;
            let x_106 : i32 = x_10.x_GLF_uniform_int_values[0].el;
            if ((x_104 < x_106)) {
            } else {
              break;
            }
            let x_110 : i32 = x_10.x_GLF_uniform_int_values[1].el;
            i_5 = x_110;
            loop {
              let x_115 : i32 = i_5;
              let x_117 : i32 = x_10.x_GLF_uniform_int_values[0].el;
              if ((x_115 < x_117)) {
              } else {
                break;
              }
              let x_121 : i32 = x_10.x_GLF_uniform_int_values[1].el;
              i_6 = x_121;
              loop {
                let x_126 : i32 = i_6;
                let x_128 : i32 = x_10.x_GLF_uniform_int_values[0].el;
                if ((x_126 < x_128)) {
                } else {
                  break;
                }
                let x_132 : i32 = x_10.x_GLF_uniform_int_values[1].el;
                i_7 = x_132;
                loop {
                  let x_137 : i32 = i_7;
                  let x_139 : i32 = x_10.x_GLF_uniform_int_values[0].el;
                  if ((x_137 < x_139)) {
                  } else {
                    break;
                  }
                  let x_143 : i32 = x_10.x_GLF_uniform_int_values[1].el;
                  i_8 = x_143;
                  loop {
                    let x_148 : i32 = i_8;
                    let x_150 : i32 = x_10.x_GLF_uniform_int_values[0].el;
                    if ((x_148 < x_150)) {
                    } else {
                      break;
                    }
                    let x_154 : i32 = x_10.x_GLF_uniform_int_values[1].el;
                    i_9 = x_154;
                    loop {
                      let x_159 : i32 = i_9;
                      let x_161 : i32 = x_10.x_GLF_uniform_int_values[0].el;
                      if ((x_159 < x_161)) {
                      } else {
                        break;
                      }
                      let x_165 : i32 = x_10.x_GLF_uniform_int_values[1].el;
                      i_10 = x_165;
                      loop {
                        let x_170 : i32 = i_10;
                        let x_172 : i32 = x_10.x_GLF_uniform_int_values[0].el;
                        if ((x_170 < x_172)) {
                        } else {
                          break;
                        }
                        let x_176 : i32 = x_10.x_GLF_uniform_int_values[1].el;
                        i_11 = x_176;
                        loop {
                          let x_181 : i32 = i_11;
                          let x_183 : i32 = x_10.x_GLF_uniform_int_values[2].el;
                          if ((x_181 < x_183)) {
                          } else {
                            break;
                          }
                          let x_187 : i32 = x_10.x_GLF_uniform_int_values[1].el;
                          i_12 = x_187;
                          loop {
                            let x_192 : i32 = i_12;
                            let x_194 : i32 = x_10.x_GLF_uniform_int_values[0].el;
                            if ((x_192 < x_194)) {
                            } else {
                              break;
                            }
                            let x_198 : i32 = x_10.x_GLF_uniform_int_values[1].el;
                            i_13 = x_198;
                            loop {
                              let x_203 : i32 = i_13;
                              let x_205 : i32 = x_10.x_GLF_uniform_int_values[0].el;
                              if ((x_203 < x_205)) {
                              } else {
                                break;
                              }
                              let x_209 : i32 = x_10.x_GLF_uniform_int_values[1].el;
                              i_14 = x_209;
                              loop {
                                let x_214 : i32 = i_14;
                                let x_216 : i32 = x_10.x_GLF_uniform_int_values[2].el;
                                if ((x_214 < x_216)) {
                                } else {
                                  break;
                                }
                                loop {
                                  let x_223 : i32 = x_GLF_global_loop_count;
                                  x_GLF_global_loop_count = (x_223 + 1);

                                  continuing {
                                    let x_225 : i32 = x_GLF_global_loop_count;
                                    let x_227 : i32 = x_10.x_GLF_uniform_int_values[3].el;
                                    break if !(x_225 < (100 - x_227));
                                  }
                                }
                                let x_231 : f32 = x_7.x_GLF_uniform_float_values[0].el;
                                let x_232 : f32 = f;
                                f = (x_232 + x_231);

                                continuing {
                                  let x_234 : i32 = i_14;
                                  i_14 = (x_234 + 1);
                                }
                              }

                              continuing {
                                let x_236 : i32 = i_13;
                                i_13 = (x_236 + 1);
                              }
                            }

                            continuing {
                              let x_238 : i32 = i_12;
                              i_12 = (x_238 + 1);
                            }
                          }

                          continuing {
                            let x_240 : i32 = i_11;
                            i_11 = (x_240 + 1);
                          }
                        }

                        continuing {
                          let x_242 : i32 = i_10;
                          i_10 = (x_242 + 1);
                        }
                      }

                      continuing {
                        let x_244 : i32 = i_9;
                        i_9 = (x_244 + 1);
                      }
                    }

                    continuing {
                      let x_246 : i32 = i_8;
                      i_8 = (x_246 + 1);
                    }
                  }

                  continuing {
                    let x_248 : i32 = i_7;
                    i_7 = (x_248 + 1);
                  }
                }

                continuing {
                  let x_250 : i32 = i_6;
                  i_6 = (x_250 + 1);
                }
              }

              continuing {
                let x_252 : i32 = i_5;
                i_5 = (x_252 + 1);
              }
            }

            continuing {
              let x_254 : i32 = i_4;
              i_4 = (x_254 + 1);
            }
          }

          continuing {
            let x_256 : i32 = i_3;
            i_3 = (x_256 + 1);
          }
        }

        continuing {
          let x_258 : i32 = i_2;
          i_2 = (x_258 + 1);
        }
      }

      continuing {
        let x_260 : i32 = i_1;
        i_1 = (x_260 + 1);
      }
    }

    continuing {
      let x_262 : i32 = i;
      i = (x_262 + 1);
    }
  }
  let x_265 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  sum = x_265;
  let x_267 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  r = x_267;
  loop {
    let x_272 : i32 = x_GLF_global_loop_count;
    if ((x_272 < 100)) {
    } else {
      break;
    }
    let x_275 : i32 = x_GLF_global_loop_count;
    x_GLF_global_loop_count = (x_275 + 1);
    let x_277 : f32 = f;
    let x_278 : f32 = sum;
    sum = (x_278 + x_277);

    continuing {
      let x_280 : i32 = r;
      r = (x_280 + 1);
    }
  }
  let x_282 : f32 = sum;
  let x_284 : f32 = x_7.x_GLF_uniform_float_values[2].el;
  if ((x_282 == x_284)) {
    let x_290 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_293 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_296 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_299 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_290), f32(x_293), f32(x_296), f32(x_299));
  } else {
    let x_303 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_304 : f32 = f32(x_303);
    x_GLF_color = vec4<f32>(x_304, x_304, x_304, x_304);
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
