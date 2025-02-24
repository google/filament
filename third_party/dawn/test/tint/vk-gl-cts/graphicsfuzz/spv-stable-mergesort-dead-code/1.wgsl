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
  let x_262 : i32 = *(f);
  k = x_262;
  let x_263 : i32 = *(f);
  i = x_263;
  let x_264 : i32 = *(mid);
  j = (x_264 + 1);
  loop {
    let x_270 : i32 = i;
    let x_271 : i32 = *(mid);
    let x_273 : i32 = j;
    let x_274 : i32 = *(to);
    if (((x_270 <= x_271) & (x_273 <= x_274))) {
    } else {
      break;
    }
    let x_278 : i32 = i;
    let x_280 : i32 = data[x_278];
    let x_281 : i32 = j;
    let x_283 : i32 = data[x_281];
    if ((x_280 < x_283)) {
      let x_288 : i32 = k;
      k = (x_288 + 1);
      let x_290 : i32 = i;
      i = (x_290 + 1);
      let x_293 : i32 = data[x_290];
      temp[x_288] = x_293;
    } else {
      let x_295 : i32 = k;
      k = (x_295 + 1);
      let x_297 : i32 = j;
      j = (x_297 + 1);
      let x_300 : i32 = data[x_297];
      temp[x_295] = x_300;
    }
  }
  loop {
    let x_306 : i32 = i;
    let x_308 : i32 = i;
    let x_309 : i32 = *(mid);
    if (((x_306 < 10) & (x_308 <= x_309))) {
    } else {
      break;
    }
    let x_313 : i32 = k;
    k = (x_313 + 1);
    let x_315 : i32 = i;
    i = (x_315 + 1);
    let x_318 : i32 = data[x_315];
    temp[x_313] = x_318;
  }
  let x_320 : i32 = *(f);
  i_1 = x_320;
  loop {
    let x_325 : i32 = i_1;
    let x_326 : i32 = *(to);
    if ((x_325 <= x_326)) {
    } else {
      break;
    }
    let x_329 : i32 = i_1;
    let x_330 : i32 = i_1;
    let x_332 : i32 = temp[x_330];
    data[x_329] = x_332;

    continuing {
      let x_334 : i32 = i_1;
      i_1 = (x_334 + 1);
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
    let x_341 : i32 = m;
    let x_342 : i32 = high;
    if ((x_341 <= x_342)) {
    } else {
      break;
    }
    let x_345 : i32 = low;
    i_2 = x_345;
    loop {
      let x_350 : i32 = i_2;
      let x_351 : i32 = high;
      if ((x_350 < x_351)) {
      } else {
        break;
      }
      let x_354 : i32 = i_2;
      f_1 = x_354;
      let x_355 : i32 = i_2;
      let x_356 : i32 = m;
      mid_1 = ((x_355 + x_356) - 1);
      let x_359 : i32 = i_2;
      let x_360 : i32 = m;
      let x_364 : i32 = high;
      to_1 = min(((x_359 + (2 * x_360)) - 1), x_364);
      let x_366 : i32 = f_1;
      param = x_366;
      let x_367 : i32 = mid_1;
      param_1 = x_367;
      let x_368 : i32 = to_1;
      param_2 = x_368;
      merge_i1_i1_i1_(&(param), &(param_1), &(param_2));

      continuing {
        let x_370 : i32 = m;
        let x_372 : i32 = i_2;
        i_2 = (x_372 + (2 * x_370));
      }
    }

    continuing {
      let x_374 : i32 = m;
      m = (2 * x_374);
    }
  }
  return;
}

fn main_1() {
  var i_3 : i32;
  var j_1 : i32;
  var grey : f32;
  let x_86 : f32 = x_28.injectionSwitch.x;
  i_3 = i32(x_86);
  loop {
    let x_92 : i32 = i_3;
    switch(x_92) {
      case 9: {
        let x_122 : i32 = i_3;
        data[x_122] = -5;
      }
      case 8: {
        let x_120 : i32 = i_3;
        data[x_120] = -4;
      }
      case 7: {
        let x_118 : i32 = i_3;
        data[x_118] = -3;
      }
      case 6: {
        let x_116 : i32 = i_3;
        data[x_116] = -2;
      }
      case 5: {
        let x_114 : i32 = i_3;
        data[x_114] = -1;
      }
      case 4: {
        let x_112 : i32 = i_3;
        data[x_112] = 0;
      }
      case 3: {
        let x_110 : i32 = i_3;
        data[x_110] = 1;
      }
      case 2: {
        let x_108 : i32 = i_3;
        data[x_108] = 2;
      }
      case 1: {
        let x_106 : i32 = i_3;
        data[x_106] = 3;
      }
      case 0: {
        let x_104 : i32 = i_3;
        data[x_104] = 4;
      }
      default: {
      }
    }
    let x_124 : i32 = i_3;
    i_3 = (x_124 + 1);

    continuing {
      let x_126 : i32 = i_3;
      break if !(x_126 < 10);
    }
  }
  j_1 = 0;
  loop {
    let x_132 : i32 = j_1;
    let x_133 : bool = (x_132 < 10);
    let x_135 : f32 = x_28.injectionSwitch.x;
    if (!((x_135 <= 1.0))) {
      grey = 1.0;
    }
    if (x_133) {
    } else {
      break;
    }
    let x_140 : i32 = j_1;
    let x_141 : i32 = j_1;
    let x_143 : i32 = data[x_141];
    temp[x_140] = x_143;

    continuing {
      let x_145 : i32 = j_1;
      j_1 = (x_145 + 1);
    }
  }
  mergeSort_();
  let x_149 : f32 = gl_FragCoord.y;
  if ((i32(x_149) < 30)) {
    let x_156 : i32 = data[0];
    grey = (0.5 + (f32(x_156) / 10.0));
  } else {
    let x_161 : f32 = gl_FragCoord.y;
    if ((i32(x_161) < 60)) {
      let x_168 : i32 = data[1];
      grey = (0.5 + (f32(x_168) / 10.0));
    } else {
      let x_173 : f32 = gl_FragCoord.y;
      if ((i32(x_173) < 90)) {
        let x_180 : i32 = data[2];
        grey = (0.5 + (f32(x_180) / 10.0));
      } else {
        let x_185 : f32 = gl_FragCoord.y;
        if ((i32(x_185) < 120)) {
          let x_192 : i32 = data[3];
          grey = (0.5 + (f32(x_192) / 10.0));
        } else {
          let x_197 : f32 = gl_FragCoord.y;
          if ((i32(x_197) < 150)) {
            discard;
          } else {
            let x_204 : f32 = gl_FragCoord.y;
            if ((i32(x_204) < 180)) {
              let x_211 : i32 = data[5];
              grey = (0.5 + (f32(x_211) / 10.0));
            } else {
              let x_216 : f32 = gl_FragCoord.y;
              if ((i32(x_216) < 210)) {
                let x_223 : i32 = data[6];
                grey = (0.5 + (f32(x_223) / 10.0));
              } else {
                let x_228 : f32 = gl_FragCoord.y;
                if ((i32(x_228) < 240)) {
                  let x_235 : i32 = data[7];
                  grey = (0.5 + (f32(x_235) / 10.0));
                } else {
                  let x_240 : f32 = gl_FragCoord.y;
                  var guard233 : bool = true;
                  if ((i32(x_240) < 270)) {
                    let x_247 : i32 = data[8];
                    grey = (0.5 + (f32(x_247) / 10.0));
                    guard233 = false;
                  } else {
                    if (guard233) {
                      let x_252 : f32 = x_28.injectionSwitch.y;
                      if (!((0.0 < x_252))) {
                        guard233 = false;
                      }
                      if (guard233) {
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
    }
  }
  let x_255 : f32 = grey;
  let x_256 : vec3<f32> = vec3<f32>(x_255, x_255, x_255);
  x_GLF_color = vec4<f32>(x_256.x, x_256.y, x_256.z, 1.0);
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
