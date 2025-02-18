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
    var x_285 : i32;
    var x_286 : i32;
    var x_305 : i32;
    var x_306 : i32;
    var x_320 : i32;
    var x_324 : i32;
    var x_339 : i32;
    var x_338 : i32;
    var x_352 : i32;
    var x_351 : i32;
    var x_366 : i32;
    var x_365 : i32;
    var x_287_phi : i32;
    var x_307_phi : i32;
    var x_328_phi : i32;
    var x_340_phi : i32;
    var x_353_phi : i32;
    var x_367_phi : i32;
    let x_261 : f32 = x_28.injectionSwitch.x;
    if ((1.0 >= x_261)) {
    } else {
      continue;
    }
    let x_266 : i32 = i;
    let x_267 : i32 = *(mid);
    let x_269 : i32 = j;
    let x_270 : i32 = *(to);
    if (((x_266 <= x_267) & (x_269 <= x_270))) {
    } else {
      break;
    }
    let x_274 : i32 = i;
    let x_276 : i32 = data[x_274];
    let x_277 : i32 = j;
    let x_279 : i32 = data[x_277];
    let x_280 : bool = (x_276 < x_279);
    if (x_280) {
      x_285 = k;
      x_287_phi = x_285;
    } else {
      x_286 = 0;
      x_287_phi = x_286;
    }
    let x_287 : i32 = x_287_phi;
    let x_288 : i32 = (x_287 + 1);
    if (x_280) {
      k = x_288;
      let x_293 : f32 = x_28.injectionSwitch.x;
      if (!((1.0 <= x_293))) {
      } else {
        continue;
      }
    }
    let x_297 : f32 = x_28.injectionSwitch.y;
    if ((x_297 >= 0.0)) {
    } else {
      continue;
    }
    let x_300 : i32 = 0;
    if (x_280) {
      x_305 = i;
      x_307_phi = x_305;
    } else {
      x_306 = 0;
      x_307_phi = x_306;
    }
    let x_307 : i32 = x_307_phi;
    let x_309 : i32 = select(x_300, x_307, x_280);
    if (x_280) {
      i = (x_309 + 1);
    }
    let x_315 : i32 = 0;
    if (x_280) {
      x_320 = data[x_309];
      let x_322 : f32 = x_28.injectionSwitch.y;
      x_328_phi = x_320;
      if (!((0.0 <= x_322))) {
        continue;
      }
    } else {
      x_324 = 0;
      let x_326 : f32 = x_28.injectionSwitch.y;
      x_328_phi = x_324;
      if (!((x_326 < 0.0))) {
      } else {
        continue;
      }
    }
    let x_328 : i32 = x_328_phi;
    if (x_280) {
      temp[x_287] = select(x_315, x_328, x_280);
    }
    if (x_280) {
      x_339 = 0;
      x_340_phi = x_339;
    } else {
      x_338 = k;
      x_340_phi = x_338;
    }
    let x_340 : i32 = x_340_phi;
    if (x_280) {
    } else {
      k = (x_340 + 1);
    }
    let x_345 : f32 = x_28.injectionSwitch.x;
    if (!((1.0 <= x_345))) {
    } else {
      continue;
    }
    if (x_280) {
      x_352 = 0;
      x_353_phi = x_352;
    } else {
      x_351 = j;
      x_353_phi = x_351;
    }
    let x_353 : i32 = x_353_phi;
    let x_355 : i32 = 0;
    let x_357 : i32 = select(x_353, x_355, x_280);
    if (x_280) {
    } else {
      j = (x_357 + 1);
    }
    if (x_280) {
      x_366 = 0;
      x_367_phi = x_366;
    } else {
      x_365 = data[x_357];
      x_367_phi = x_365;
    }
    let x_367 : i32 = x_367_phi;
    if (x_280) {
    } else {
      temp[x_340] = x_367;
    }
  }
  loop {
    let x_376 : i32 = i;
    let x_378 : i32 = i;
    let x_379 : i32 = *(mid);
    if (((x_376 < 10) & (x_378 <= x_379))) {
    } else {
      break;
    }
    let x_383 : i32 = k;
    k = (x_383 + 1);
    let x_385 : i32 = i;
    i = (x_385 + 1);
    let x_388 : i32 = data[x_385];
    temp[x_383] = x_388;
  }
  let x_390 : i32 = *(f);
  i_1 = x_390;
  loop {
    let x_395 : i32 = i_1;
    let x_396 : i32 = *(to);
    if ((x_395 <= x_396)) {
    } else {
      break;
    }
    let x_399 : i32 = i_1;
    let x_400 : i32 = i_1;
    let x_402 : i32 = temp[x_400];
    data[x_399] = x_402;

    continuing {
      let x_404 : i32 = i_1;
      i_1 = (x_404 + 1);
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
    let x_411 : i32 = m;
    let x_412 : i32 = high;
    if ((x_411 <= x_412)) {
    } else {
      break;
    }
    let x_415 : i32 = low;
    i_2 = x_415;
    loop {
      let x_420 : i32 = i_2;
      let x_421 : i32 = high;
      if ((x_420 < x_421)) {
      } else {
        break;
      }
      let x_424 : i32 = i_2;
      f_1 = x_424;
      let x_425 : i32 = i_2;
      let x_426 : i32 = m;
      mid_1 = ((x_425 + x_426) - 1);
      let x_429 : i32 = i_2;
      let x_430 : i32 = m;
      let x_434 : i32 = high;
      to_1 = min(((x_429 + (2 * x_430)) - 1), x_434);
      let x_436 : i32 = f_1;
      param = x_436;
      let x_437 : i32 = mid_1;
      param_1 = x_437;
      let x_438 : i32 = to_1;
      param_2 = x_438;
      merge_i1_i1_i1_(&(param), &(param_1), &(param_2));

      continuing {
        let x_440 : i32 = m;
        let x_442 : i32 = i_2;
        i_2 = (x_442 + (2 * x_440));
      }
    }

    continuing {
      let x_444 : i32 = m;
      m = (2 * x_444);
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
