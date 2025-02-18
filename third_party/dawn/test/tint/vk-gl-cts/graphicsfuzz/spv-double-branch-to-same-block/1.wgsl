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
  let x_255 : i32 = *(f);
  k = x_255;
  let x_256 : i32 = *(f);
  i = x_256;
  let x_257 : i32 = *(mid);
  j = (x_257 + 1);
  loop {
    let x_263 : i32 = i;
    let x_264 : i32 = *(mid);
    let x_266 : i32 = j;
    let x_267 : i32 = *(to);
    if (((x_263 <= x_264) & (x_266 <= x_267))) {
    } else {
      break;
    }
    let x_271 : i32 = i;
    let x_273 : i32 = data[x_271];
    let x_274 : i32 = j;
    let x_276 : i32 = data[x_274];
    if ((x_273 < x_276)) {
      let x_281 : i32 = k;
      k = (x_281 + 1);
      let x_283 : i32 = i;
      i = (x_283 + 1);
      let x_286 : i32 = data[x_283];
      temp[x_281] = x_286;
    } else {
      let x_288 : i32 = k;
      k = (x_288 + 1);
      let x_290 : i32 = j;
      j = (x_290 + 1);
      let x_293 : i32 = data[x_290];
      temp[x_288] = x_293;
    }
  }
  loop {
    let x_299 : i32 = i;
    let x_301 : i32 = i;
    let x_302 : i32 = *(mid);
    if (((x_299 < 10) & (x_301 <= x_302))) {
    } else {
      break;
    }
    let x_306 : i32 = k;
    k = (x_306 + 1);
    let x_308 : i32 = i;
    i = (x_308 + 1);
    let x_311 : i32 = data[x_308];
    temp[x_306] = x_311;
  }
  let x_313 : i32 = *(f);
  i_1 = x_313;
  loop {
    let x_318 : i32 = i_1;
    let x_319 : i32 = *(to);
    if ((x_318 <= x_319)) {
    } else {
      break;
    }
    let x_322 : i32 = i_1;
    let x_323 : i32 = i_1;
    let x_325 : i32 = temp[x_323];
    data[x_322] = x_325;

    continuing {
      let x_327 : i32 = i_1;
      i_1 = (x_327 + 1);
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
    let x_334 : i32 = m;
    let x_335 : i32 = high;
    if ((x_334 <= x_335)) {
    } else {
      break;
    }
    let x_338 : i32 = low;
    i_2 = x_338;
    loop {
      let x_343 : i32 = i_2;
      let x_344 : i32 = high;
      if ((x_343 < x_344)) {
      } else {
        break;
      }
      let x_347 : i32 = i_2;
      f_1 = x_347;
      let x_348 : i32 = i_2;
      let x_349 : i32 = m;
      mid_1 = ((x_348 + x_349) - 1);
      let x_352 : i32 = i_2;
      let x_353 : i32 = m;
      let x_357 : i32 = high;
      to_1 = min(((x_352 + (2 * x_353)) - 1), x_357);
      let x_359 : i32 = f_1;
      param = x_359;
      let x_360 : i32 = mid_1;
      param_1 = x_360;
      let x_361 : i32 = to_1;
      param_2 = x_361;
      merge_i1_i1_i1_(&(param), &(param_1), &(param_2));

      continuing {
        let x_363 : i32 = m;
        let x_365 : i32 = i_2;
        i_2 = (x_365 + (2 * x_363));
      }
    }

    continuing {
      let x_367 : i32 = m;
      m = (2 * x_367);
    }
  }
  return;
}

fn main_1() {
  var i_3 : i32;
  var j_1 : i32;
  var grey : f32;
  let x_88 : f32 = x_28.injectionSwitch.x;
  i_3 = i32(x_88);
  loop {
    let x_94 : i32 = i_3;
    switch(x_94) {
      case 9: {
        let x_124 : i32 = i_3;
        data[x_124] = -5;
        if (true) {
        } else {
          continue;
        }
      }
      case 8: {
        let x_122 : i32 = i_3;
        data[x_122] = -4;
      }
      case 7: {
        let x_120 : i32 = i_3;
        data[x_120] = -3;
      }
      case 6: {
        let x_118 : i32 = i_3;
        data[x_118] = -2;
      }
      case 5: {
        let x_116 : i32 = i_3;
        data[x_116] = -1;
      }
      case 4: {
        let x_114 : i32 = i_3;
        data[x_114] = 0;
      }
      case 3: {
        let x_112 : i32 = i_3;
        data[x_112] = 1;
      }
      case 2: {
        let x_110 : i32 = i_3;
        data[x_110] = 2;
      }
      case 1: {
        let x_108 : i32 = i_3;
        data[x_108] = 3;
      }
      case 0: {
        let x_106 : i32 = i_3;
        data[x_106] = 4;
      }
      default: {
      }
    }
    let x_126 : i32 = i_3;
    i_3 = (x_126 + 1);

    continuing {
      let x_128 : i32 = i_3;
      break if !(x_128 < 10);
    }
  }
  j_1 = 0;
  loop {
    let x_134 : i32 = j_1;
    if ((x_134 < 10)) {
    } else {
      break;
    }
    let x_137 : i32 = j_1;
    let x_138 : i32 = j_1;
    let x_140 : i32 = data[x_138];
    temp[x_137] = x_140;

    continuing {
      let x_142 : i32 = j_1;
      j_1 = (x_142 + 1);
    }
  }
  mergeSort_();
  let x_146 : f32 = gl_FragCoord.y;
  if ((i32(x_146) < 30)) {
    let x_153 : i32 = data[0];
    grey = (0.5 + (f32(x_153) / 10.0));
  } else {
    let x_158 : f32 = gl_FragCoord.y;
    if ((i32(x_158) < 60)) {
      let x_165 : i32 = data[1];
      grey = (0.5 + (f32(x_165) / 10.0));
    } else {
      let x_170 : f32 = gl_FragCoord.y;
      if ((i32(x_170) < 90)) {
        let x_177 : i32 = data[2];
        grey = (0.5 + (f32(x_177) / 10.0));
      } else {
        let x_182 : f32 = gl_FragCoord.y;
        if ((i32(x_182) < 120)) {
          let x_189 : i32 = data[3];
          grey = (0.5 + (f32(x_189) / 10.0));
        } else {
          let x_194 : f32 = gl_FragCoord.y;
          if ((i32(x_194) < 150)) {
            discard;
          } else {
            let x_201 : f32 = gl_FragCoord.y;
            if ((i32(x_201) < 180)) {
              let x_208 : i32 = data[5];
              grey = (0.5 + (f32(x_208) / 10.0));
            } else {
              let x_213 : f32 = gl_FragCoord.y;
              if ((i32(x_213) < 210)) {
                let x_220 : i32 = data[6];
                grey = (0.5 + (f32(x_220) / 10.0));
              } else {
                let x_225 : f32 = gl_FragCoord.y;
                if ((i32(x_225) < 240)) {
                  let x_232 : i32 = data[7];
                  grey = (0.5 + (f32(x_232) / 10.0));
                } else {
                  let x_237 : f32 = gl_FragCoord.y;
                  if ((i32(x_237) < 270)) {
                    let x_244 : i32 = data[8];
                    grey = (0.5 + (f32(x_244) / 10.0));
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
  let x_248 : f32 = grey;
  let x_249 : vec3<f32> = vec3<f32>(x_248, x_248, x_248);
  x_GLF_color = vec4<f32>(x_249.x, x_249.y, x_249.z, 1.0);
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
