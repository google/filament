struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> data : array<i32, 10u>;

var<private> temp : array<i32, 10u>;

@group(0) @binding(0) var<uniform> x_28 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

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
    var x_283 : i32;
    var x_284 : i32;
    var x_303 : i32;
    var x_304 : i32;
    var x_318 : i32;
    var x_322 : i32;
    var x_337 : i32;
    var x_336 : i32;
    var x_350 : i32;
    var x_349 : i32;
    var x_364 : i32;
    var x_363 : i32;
    var x_285_phi : i32;
    var x_305_phi : i32;
    var x_326_phi : i32;
    var x_338_phi : i32;
    var x_351_phi : i32;
    var x_365_phi : i32;
    if ((1.0 >= 0.0)) {
    } else {
      continue;
    }
    let x_264 : i32 = i;
    let x_265 : i32 = *(mid);
    let x_267 : i32 = j;
    let x_268 : i32 = *(to);
    if (((x_264 <= x_265) & (x_267 <= x_268))) {
    } else {
      break;
    }
    let x_272 : i32 = i;
    let x_274 : i32 = data[x_272];
    let x_275 : i32 = j;
    let x_277 : i32 = data[x_275];
    let x_278 : bool = (x_274 < x_277);
    if (x_278) {
      x_283 = k;
      x_285_phi = x_283;
    } else {
      x_284 = 0;
      x_285_phi = x_284;
    }
    let x_285 : i32 = x_285_phi;
    let x_286 : i32 = (x_285 + 1);
    if (x_278) {
      k = x_286;
      let x_291 : f32 = x_28.injectionSwitch.x;
      if (!((1.0 <= x_291))) {
      } else {
        continue;
      }
    }
    let x_295 : f32 = x_28.injectionSwitch.y;
    if ((x_295 >= 0.0)) {
    } else {
      continue;
    }
    let x_298 : i32 = 0;
    if (x_278) {
      x_303 = i;
      x_305_phi = x_303;
    } else {
      x_304 = 0;
      x_305_phi = x_304;
    }
    let x_305 : i32 = x_305_phi;
    let x_307 : i32 = select(x_298, x_305, x_278);
    if (x_278) {
      i = (x_307 + 1);
    }
    let x_313 : i32 = 0;
    if (x_278) {
      x_318 = data[x_307];
      let x_320 : f32 = x_28.injectionSwitch.y;
      x_326_phi = x_318;
      if (!((0.0 <= x_320))) {
        continue;
      }
    } else {
      x_322 = 0;
      let x_324 : f32 = x_28.injectionSwitch.y;
      x_326_phi = x_322;
      if (!((x_324 < 0.0))) {
      } else {
        continue;
      }
    }
    let x_326 : i32 = x_326_phi;
    if (x_278) {
      temp[x_285] = select(x_313, x_326, x_278);
    }
    if (x_278) {
      x_337 = 0;
      x_338_phi = x_337;
    } else {
      x_336 = k;
      x_338_phi = x_336;
    }
    let x_338 : i32 = x_338_phi;
    if (x_278) {
    } else {
      k = (x_338 + 1);
    }
    let x_343 : f32 = x_28.injectionSwitch.x;
    if (!((1.0 <= x_343))) {
    } else {
      continue;
    }
    if (x_278) {
      x_350 = 0;
      x_351_phi = x_350;
    } else {
      x_349 = j;
      x_351_phi = x_349;
    }
    let x_351 : i32 = x_351_phi;
    let x_353 : i32 = 0;
    let x_355 : i32 = select(x_351, x_353, x_278);
    if (x_278) {
    } else {
      j = (x_355 + 1);
    }
    if (x_278) {
      x_364 = 0;
      x_365_phi = x_364;
    } else {
      x_363 = data[x_355];
      x_365_phi = x_363;
    }
    let x_365 : i32 = x_365_phi;
    if (x_278) {
    } else {
      temp[x_338] = x_365;
    }
  }
  loop {
    let x_374 : i32 = i;
    let x_376 : i32 = i;
    let x_377 : i32 = *(mid);
    if (((x_374 < 10) & (x_376 <= x_377))) {
    } else {
      break;
    }
    let x_381 : i32 = k;
    k = (x_381 + 1);
    let x_383 : i32 = i;
    i = (x_383 + 1);
    let x_386 : i32 = data[x_383];
    temp[x_381] = x_386;
  }
  let x_388 : i32 = *(f);
  i_1 = x_388;
  loop {
    let x_393 : i32 = i_1;
    let x_394 : i32 = *(to);
    if ((x_393 <= x_394)) {
    } else {
      break;
    }
    let x_397 : i32 = i_1;
    let x_398 : i32 = i_1;
    let x_400 : i32 = temp[x_398];
    data[x_397] = x_400;

    continuing {
      let x_402 : i32 = i_1;
      i_1 = (x_402 + 1);
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
    let x_409 : i32 = m;
    let x_410 : i32 = high;
    if ((x_409 <= x_410)) {
    } else {
      break;
    }
    let x_413 : i32 = low;
    i_2 = x_413;
    loop {
      let x_418 : i32 = i_2;
      let x_419 : i32 = high;
      if ((x_418 < x_419)) {
      } else {
        break;
      }
      let x_422 : i32 = i_2;
      f_1 = x_422;
      let x_423 : i32 = i_2;
      let x_424 : i32 = m;
      mid_1 = ((x_423 + x_424) - 1);
      let x_427 : i32 = i_2;
      let x_428 : i32 = m;
      let x_432 : i32 = high;
      to_1 = min(((x_427 + (2 * x_428)) - 1), x_432);
      let x_434 : i32 = f_1;
      param = x_434;
      let x_435 : i32 = mid_1;
      param_1 = x_435;
      let x_436 : i32 = to_1;
      param_2 = x_436;
      merge_i1_i1_i1_(&(param), &(param_1), &(param_2));

      continuing {
        let x_438 : i32 = m;
        let x_440 : i32 = i_2;
        i_2 = (x_440 + (2 * x_438));
      }
    }

    continuing {
      let x_442 : i32 = m;
      m = (2 * x_442);
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
