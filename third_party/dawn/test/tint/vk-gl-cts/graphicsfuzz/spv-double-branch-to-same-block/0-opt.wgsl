struct buf0 {
  injectionSwitch : vec2<f32>,
}

struct buf1 {
  resolution : vec2<f32>,
}

var<private> data : array<i32, 10u>;

var<private> temp : array<i32, 10u>;

@group(0) @binding(0) var<uniform> x_28 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(1) var<uniform> x_32 : buf1;

fn merge_i1_i1_i1_(f : ptr<function, i32>, mid : ptr<function, i32>, to : ptr<function, i32>) {
  var k : i32;
  var i : i32;
  var j : i32;
  var i_1 : i32;
  let x_254 : i32 = *(f);
  k = x_254;
  let x_255 : i32 = *(f);
  i = x_255;
  let x_256 : i32 = *(mid);
  j = (x_256 + 1);
  loop {
    let x_262 : i32 = i;
    let x_263 : i32 = *(mid);
    let x_265 : i32 = j;
    let x_266 : i32 = *(to);
    if (((x_262 <= x_263) & (x_265 <= x_266))) {
    } else {
      break;
    }
    let x_270 : i32 = i;
    let x_272 : i32 = data[x_270];
    let x_273 : i32 = j;
    let x_275 : i32 = data[x_273];
    if ((x_272 < x_275)) {
      let x_280 : i32 = k;
      k = (x_280 + 1);
      let x_282 : i32 = i;
      i = (x_282 + 1);
      let x_285 : i32 = data[x_282];
      temp[x_280] = x_285;
    } else {
      let x_287 : i32 = k;
      k = (x_287 + 1);
      let x_289 : i32 = j;
      j = (x_289 + 1);
      let x_292 : i32 = data[x_289];
      temp[x_287] = x_292;
    }
  }
  loop {
    let x_298 : i32 = i;
    let x_300 : i32 = i;
    let x_301 : i32 = *(mid);
    if (((x_298 < 10) & (x_300 <= x_301))) {
    } else {
      break;
    }
    let x_305 : i32 = k;
    k = (x_305 + 1);
    let x_307 : i32 = i;
    i = (x_307 + 1);
    let x_310 : i32 = data[x_307];
    temp[x_305] = x_310;
  }
  let x_312 : i32 = *(f);
  i_1 = x_312;
  loop {
    let x_317 : i32 = i_1;
    let x_318 : i32 = *(to);
    if ((x_317 <= x_318)) {
    } else {
      break;
    }
    let x_321 : i32 = i_1;
    let x_322 : i32 = i_1;
    let x_324 : i32 = temp[x_322];
    data[x_321] = x_324;

    continuing {
      let x_326 : i32 = i_1;
      i_1 = (x_326 + 1);
    }
  }
  return;
}

fn mergeSort_() {
  var low : i32;
  var high : i32;
  var m : i32;
  var i_2 : i32;
  var f_1 : i32;
  var mid_1 : i32;
  var to_1 : i32;
  var param : i32;
  var param_1 : i32;
  var param_2 : i32;
  low = 0;
  high = 9;
  m = 1;
  loop {
    let x_333 : i32 = m;
    let x_334 : i32 = high;
    if ((x_333 <= x_334)) {
    } else {
      break;
    }
    let x_337 : i32 = low;
    i_2 = x_337;
    loop {
      let x_342 : i32 = i_2;
      let x_343 : i32 = high;
      if ((x_342 < x_343)) {
      } else {
        break;
      }
      let x_346 : i32 = i_2;
      f_1 = x_346;
      let x_347 : i32 = i_2;
      let x_348 : i32 = m;
      mid_1 = ((x_347 + x_348) - 1);
      let x_351 : i32 = i_2;
      let x_352 : i32 = m;
      let x_356 : i32 = high;
      to_1 = min(((x_351 + (2 * x_352)) - 1), x_356);
      let x_358 : i32 = f_1;
      param = x_358;
      let x_359 : i32 = mid_1;
      param_1 = x_359;
      let x_360 : i32 = to_1;
      param_2 = x_360;
      merge_i1_i1_i1_(&(param), &(param_1), &(param_2));

      continuing {
        let x_362 : i32 = m;
        let x_364 : i32 = i_2;
        i_2 = (x_364 + (2 * x_362));
      }
    }

    continuing {
      let x_366 : i32 = m;
      m = (2 * x_366);
    }
  }
  return;
}

fn main_1() {
  var i_3 : i32;
  var j_1 : i32;
  var grey : f32;
  let x_87 : f32 = x_28.injectionSwitch.x;
  i_3 = i32(x_87);
  loop {
    let x_93 : i32 = i_3;
    switch(x_93) {
      case 9: {
        let x_123 : i32 = i_3;
        data[x_123] = -5;
      }
      case 8: {
        let x_121 : i32 = i_3;
        data[x_121] = -4;
      }
      case 7: {
        let x_119 : i32 = i_3;
        data[x_119] = -3;
      }
      case 6: {
        let x_117 : i32 = i_3;
        data[x_117] = -2;
      }
      case 5: {
        let x_115 : i32 = i_3;
        data[x_115] = -1;
      }
      case 4: {
        let x_113 : i32 = i_3;
        data[x_113] = 0;
      }
      case 3: {
        let x_111 : i32 = i_3;
        data[x_111] = 1;
      }
      case 2: {
        let x_109 : i32 = i_3;
        data[x_109] = 2;
      }
      case 1: {
        let x_107 : i32 = i_3;
        data[x_107] = 3;
      }
      case 0: {
        let x_105 : i32 = i_3;
        data[x_105] = 4;
      }
      default: {
      }
    }
    let x_125 : i32 = i_3;
    i_3 = (x_125 + 1);

    continuing {
      let x_127 : i32 = i_3;
      break if !(x_127 < 10);
    }
  }
  j_1 = 0;
  loop {
    let x_133 : i32 = j_1;
    if ((x_133 < 10)) {
    } else {
      break;
    }
    let x_136 : i32 = j_1;
    let x_137 : i32 = j_1;
    let x_139 : i32 = data[x_137];
    temp[x_136] = x_139;

    continuing {
      let x_141 : i32 = j_1;
      j_1 = (x_141 + 1);
    }
  }
  mergeSort_();
  let x_145 : f32 = gl_FragCoord.y;
  if ((i32(x_145) < 30)) {
    let x_152 : i32 = data[0];
    grey = (0.5 + (f32(x_152) / 10.0));
  } else {
    let x_157 : f32 = gl_FragCoord.y;
    if ((i32(x_157) < 60)) {
      let x_164 : i32 = data[1];
      grey = (0.5 + (f32(x_164) / 10.0));
    } else {
      let x_169 : f32 = gl_FragCoord.y;
      if ((i32(x_169) < 90)) {
        let x_176 : i32 = data[2];
        grey = (0.5 + (f32(x_176) / 10.0));
      } else {
        let x_181 : f32 = gl_FragCoord.y;
        if ((i32(x_181) < 120)) {
          let x_188 : i32 = data[3];
          grey = (0.5 + (f32(x_188) / 10.0));
        } else {
          let x_193 : f32 = gl_FragCoord.y;
          if ((i32(x_193) < 150)) {
            discard;
          } else {
            let x_200 : f32 = gl_FragCoord.y;
            if ((i32(x_200) < 180)) {
              let x_207 : i32 = data[5];
              grey = (0.5 + (f32(x_207) / 10.0));
            } else {
              let x_212 : f32 = gl_FragCoord.y;
              if ((i32(x_212) < 210)) {
                let x_219 : i32 = data[6];
                grey = (0.5 + (f32(x_219) / 10.0));
              } else {
                let x_224 : f32 = gl_FragCoord.y;
                if ((i32(x_224) < 240)) {
                  let x_231 : i32 = data[7];
                  grey = (0.5 + (f32(x_231) / 10.0));
                } else {
                  let x_236 : f32 = gl_FragCoord.y;
                  if ((i32(x_236) < 270)) {
                    let x_243 : i32 = data[8];
                    grey = (0.5 + (f32(x_243) / 10.0));
                  } else {
                    discard;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  let x_247 : f32 = grey;
  let x_248 : vec3<f32> = vec3<f32>(x_247, x_247, x_247);
  x_GLF_color = vec4<f32>(x_248.x, x_248.y, x_248.z, 1.0);
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
