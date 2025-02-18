struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> data : array<i32, 10u>;

var<private> temp : array<i32, 10u>;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_34 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn merge_i1_i1_i1_(f : ptr<function, i32>, mid : ptr<function, i32>, to : ptr<function, i32>) {
  var k : i32;
  var i : i32;
  var j : i32;
  var i_1 : i32;
  let x_260 : i32 = *(f);
  k = x_260;
  let x_261 : i32 = *(f);
  i = x_261;
  let x_262 : i32 = *(mid);
  j = (x_262 + 1);
  loop {
    let x_268 : i32 = i;
    let x_269 : i32 = *(mid);
    let x_271 : i32 = j;
    let x_272 : i32 = *(to);
    if (((x_268 <= x_269) & (x_271 <= x_272))) {
    } else {
      break;
    }
    let x_276 : i32 = i;
    let x_278 : i32 = data[x_276];
    let x_279 : i32 = j;
    let x_281 : i32 = data[x_279];
    if ((x_278 < x_281)) {
      let x_286 : i32 = k;
      k = (x_286 + 1);
      let x_288 : i32 = i;
      i = (x_288 + 1);
      let x_291 : i32 = data[x_288];
      temp[x_286] = x_291;
    } else {
      let x_293 : i32 = k;
      k = (x_293 + 1);
      let x_295 : i32 = j;
      j = (x_295 + 1);
      let x_298 : i32 = data[x_295];
      temp[x_293] = x_298;
    }
  }
  loop {
    let x_304 : i32 = i;
    let x_306 : i32 = i;
    let x_307 : i32 = *(mid);
    if (((x_304 < 10) & (x_306 <= x_307))) {
    } else {
      break;
    }
    let x_311 : i32 = k;
    k = (x_311 + 1);
    let x_313 : i32 = i;
    i = (x_313 + 1);
    let x_316 : i32 = data[x_313];
    temp[x_311] = x_316;
  }
  let x_318 : i32 = *(f);
  i_1 = x_318;
  loop {
    let x_323 : i32 = i_1;
    let x_324 : i32 = *(to);
    if ((x_323 <= x_324)) {
    } else {
      break;
    }
    let x_327 : i32 = i_1;
    let x_328 : i32 = i_1;
    let x_330 : i32 = temp[x_328];
    data[x_327] = x_330;

    continuing {
      let x_332 : i32 = i_1;
      i_1 = (x_332 + 1);
    }
  }
  return;
}

fn func_i1_i1_(m : ptr<function, i32>, high : ptr<function, i32>) -> i32 {
  var x : i32;
  var x_335 : i32;
  var x_336 : i32;
  let x_338 : f32 = gl_FragCoord.x;
  if ((x_338 >= 0.0)) {
    if (false) {
      let x_346 : i32 = *(high);
      x_336 = (x_346 << bitcast<u32>(0));
    } else {
      x_336 = 4;
    }
    let x_348 : i32 = x_336;
    x_335 = (1 << bitcast<u32>(x_348));
  } else {
    x_335 = 1;
  }
  let x_350 : i32 = x_335;
  x = x_350;
  let x_351 : i32 = x;
  x = (x_351 >> bitcast<u32>(4));
  let x_353 : i32 = *(m);
  let x_355 : i32 = *(m);
  let x_357 : i32 = *(m);
  let x_359 : i32 = x;
  return clamp((2 * x_353), (2 * x_355), ((2 * x_357) / x_359));
}

fn mergeSort_() {
  var low : i32;
  var high_1 : i32;
  var m_1 : i32;
  var i_2 : i32;
  var f_1 : i32;
  var mid_1 : i32;
  var to_1 : i32;
  var param : i32;
  var param_1 : i32;
  var param_2 : i32;
  var param_3 : i32;
  var param_4 : i32;
  low = 0;
  high_1 = 9;
  m_1 = 1;
  loop {
    let x_367 : i32 = m_1;
    let x_368 : i32 = high_1;
    if ((x_367 <= x_368)) {
    } else {
      break;
    }
    let x_371 : i32 = low;
    i_2 = x_371;
    loop {
      let x_376 : i32 = i_2;
      let x_377 : i32 = high_1;
      if ((x_376 < x_377)) {
      } else {
        break;
      }
      let x_380 : i32 = i_2;
      f_1 = x_380;
      let x_381 : i32 = i_2;
      let x_382 : i32 = m_1;
      mid_1 = ((x_381 + x_382) - 1);
      let x_385 : i32 = i_2;
      let x_386 : i32 = m_1;
      let x_390 : i32 = high_1;
      to_1 = min(((x_385 + (2 * x_386)) - 1), x_390);
      let x_392 : i32 = f_1;
      param = x_392;
      let x_393 : i32 = mid_1;
      param_1 = x_393;
      let x_394 : i32 = to_1;
      param_2 = x_394;
      merge_i1_i1_i1_(&(param), &(param_1), &(param_2));

      continuing {
        let x_396 : i32 = m_1;
        param_3 = x_396;
        let x_397 : i32 = high_1;
        param_4 = x_397;
        let x_398 : i32 = func_i1_i1_(&(param_3), &(param_4));
        let x_399 : i32 = i_2;
        i_2 = (x_399 + x_398);
      }
    }

    continuing {
      let x_401 : i32 = m_1;
      m_1 = (2 * x_401);
    }
  }
  return;
}

fn main_1() {
  var i_3 : i32;
  var j_1 : i32;
  var grey : f32;
  let x_93 : f32 = x_34.injectionSwitch.x;
  i_3 = i32(x_93);
  loop {
    let x_99 : i32 = i_3;
    switch(x_99) {
      case 9: {
        let x_129 : i32 = i_3;
        data[x_129] = -5;
      }
      case 8: {
        let x_127 : i32 = i_3;
        data[x_127] = -4;
      }
      case 7: {
        let x_125 : i32 = i_3;
        data[x_125] = -3;
      }
      case 6: {
        let x_123 : i32 = i_3;
        data[x_123] = -2;
      }
      case 5: {
        let x_121 : i32 = i_3;
        data[x_121] = -1;
      }
      case 4: {
        let x_119 : i32 = i_3;
        data[x_119] = 0;
      }
      case 3: {
        let x_117 : i32 = i_3;
        data[x_117] = 1;
      }
      case 2: {
        let x_115 : i32 = i_3;
        data[x_115] = 2;
      }
      case 1: {
        let x_113 : i32 = i_3;
        data[x_113] = 3;
      }
      case 0: {
        let x_111 : i32 = i_3;
        data[x_111] = 4;
      }
      default: {
      }
    }
    let x_131 : i32 = i_3;
    i_3 = (x_131 + 1);

    continuing {
      let x_133 : i32 = i_3;
      break if !(x_133 < 10);
    }
  }
  j_1 = 0;
  loop {
    let x_139 : i32 = j_1;
    if ((x_139 < 10)) {
    } else {
      break;
    }
    let x_142 : i32 = j_1;
    let x_143 : i32 = j_1;
    let x_145 : i32 = data[x_143];
    temp[x_142] = x_145;

    continuing {
      let x_147 : i32 = j_1;
      j_1 = (x_147 + 1);
    }
  }
  mergeSort_();
  let x_151 : f32 = gl_FragCoord.y;
  if ((i32(x_151) < 30)) {
    let x_158 : i32 = data[0];
    grey = (0.5 + (f32(x_158) / 10.0));
  } else {
    let x_163 : f32 = gl_FragCoord.y;
    if ((i32(x_163) < 60)) {
      let x_170 : i32 = data[1];
      grey = (0.5 + (f32(x_170) / 10.0));
    } else {
      let x_175 : f32 = gl_FragCoord.y;
      if ((i32(x_175) < 90)) {
        let x_182 : i32 = data[2];
        grey = (0.5 + (f32(x_182) / 10.0));
      } else {
        let x_187 : f32 = gl_FragCoord.y;
        if ((i32(x_187) < 120)) {
          let x_194 : i32 = data[3];
          grey = (0.5 + (f32(x_194) / 10.0));
        } else {
          let x_199 : f32 = gl_FragCoord.y;
          if ((i32(x_199) < 150)) {
            discard;
          } else {
            let x_206 : f32 = gl_FragCoord.y;
            if ((i32(x_206) < 180)) {
              let x_213 : i32 = data[5];
              grey = (0.5 + (f32(x_213) / 10.0));
            } else {
              let x_218 : f32 = gl_FragCoord.y;
              if ((i32(x_218) < 210)) {
                let x_225 : i32 = data[6];
                grey = (0.5 + (f32(x_225) / 10.0));
              } else {
                let x_230 : f32 = gl_FragCoord.y;
                if ((i32(x_230) < 240)) {
                  let x_237 : i32 = data[7];
                  grey = (0.5 + (f32(x_237) / 10.0));
                } else {
                  let x_242 : f32 = gl_FragCoord.y;
                  if ((i32(x_242) < 270)) {
                    let x_249 : i32 = data[8];
                    grey = (0.5 + (f32(x_249) / 10.0));
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
  let x_253 : f32 = grey;
  let x_254 : vec3<f32> = vec3<f32>(x_253, x_253, x_253);
  x_GLF_color = vec4<f32>(x_254.x, x_254.y, x_254.z, 1.0);
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
