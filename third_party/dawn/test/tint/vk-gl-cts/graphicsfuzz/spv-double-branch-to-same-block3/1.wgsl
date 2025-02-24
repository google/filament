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
  let x_256 : i32 = *(f);
  k = x_256;
  let x_257 : i32 = *(f);
  i = x_257;
  let x_258 : i32 = *(mid);
  j = (x_258 + 1);
  loop {
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
    if ((x_274 < x_277)) {
      let x_282 : i32 = k;
      k = (x_282 + 1);
      let x_284 : i32 = i;
      i = (x_284 + 1);
      let x_287 : i32 = data[x_284];
      temp[x_282] = x_287;
    } else {
      let x_289 : i32 = k;
      k = (x_289 + 1);
      let x_291 : i32 = j;
      j = (x_291 + 1);
      let x_294 : i32 = data[x_291];
      temp[x_289] = x_294;
    }
  }
  loop {
    if (!((256.0 < 1.0))) {
    } else {
      continue;
    }
    let x_301 : i32 = i;
    let x_303 : i32 = i;
    let x_304 : i32 = *(mid);
    if (((x_301 < 10) & (x_303 <= x_304))) {
    } else {
      break;
    }
    let x_309 : i32 = k;
    k = (x_309 + 1);
    let x_311 : i32 = i;
    i = (x_311 + 1);
    let x_314 : i32 = data[x_311];
    temp[x_309] = x_314;
  }
  let x_316 : i32 = *(f);
  i_1 = x_316;
  loop {
    let x_321 : i32 = i_1;
    let x_322 : i32 = *(to);
    if ((x_321 <= x_322)) {
    } else {
      break;
    }
    let x_325 : i32 = i_1;
    let x_326 : i32 = i_1;
    let x_328 : i32 = temp[x_326];
    data[x_325] = x_328;

    continuing {
      let x_330 : i32 = i_1;
      i_1 = (x_330 + 1);
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
    let x_337 : i32 = m;
    let x_338 : i32 = high;
    if ((x_337 <= x_338)) {
    } else {
      break;
    }
    let x_341 : i32 = low;
    i_2 = x_341;
    loop {
      let x_346 : i32 = i_2;
      let x_347 : i32 = high;
      if ((x_346 < x_347)) {
      } else {
        break;
      }
      let x_350 : i32 = i_2;
      f_1 = x_350;
      let x_351 : i32 = i_2;
      let x_352 : i32 = m;
      mid_1 = ((x_351 + x_352) - 1);
      let x_355 : i32 = i_2;
      let x_356 : i32 = m;
      let x_360 : i32 = high;
      to_1 = min(((x_355 + (2 * x_356)) - 1), x_360);
      let x_362 : i32 = f_1;
      param = x_362;
      let x_363 : i32 = mid_1;
      param_1 = x_363;
      let x_364 : i32 = to_1;
      param_2 = x_364;
      merge_i1_i1_i1_(&(param), &(param_1), &(param_2));

      continuing {
        let x_366 : i32 = m;
        let x_368 : i32 = i_2;
        i_2 = (x_368 + (2 * x_366));
      }
    }

    continuing {
      let x_370 : i32 = m;
      m = (2 * x_370);
    }
  }
  return;
}

fn main_1() {
  var i_3 : i32;
  var j_1 : i32;
  var grey : f32;
  let x_89 : f32 = x_28.injectionSwitch.x;
  i_3 = i32(x_89);
  loop {
    let x_95 : i32 = i_3;
    switch(x_95) {
      case 9: {
        let x_125 : i32 = i_3;
        data[x_125] = -5;
      }
      case 8: {
        let x_123 : i32 = i_3;
        data[x_123] = -4;
      }
      case 7: {
        let x_121 : i32 = i_3;
        data[x_121] = -3;
      }
      case 6: {
        let x_119 : i32 = i_3;
        data[x_119] = -2;
      }
      case 5: {
        let x_117 : i32 = i_3;
        data[x_117] = -1;
      }
      case 4: {
        let x_115 : i32 = i_3;
        data[x_115] = 0;
      }
      case 3: {
        let x_113 : i32 = i_3;
        data[x_113] = 1;
      }
      case 2: {
        let x_111 : i32 = i_3;
        data[x_111] = 2;
      }
      case 1: {
        let x_109 : i32 = i_3;
        data[x_109] = 3;
      }
      case 0: {
        let x_107 : i32 = i_3;
        data[x_107] = 4;
      }
      default: {
      }
    }
    let x_127 : i32 = i_3;
    i_3 = (x_127 + 1);

    continuing {
      let x_129 : i32 = i_3;
      break if !(x_129 < 10);
    }
  }
  j_1 = 0;
  loop {
    let x_135 : i32 = j_1;
    if ((x_135 < 10)) {
    } else {
      break;
    }
    let x_138 : i32 = j_1;
    let x_139 : i32 = j_1;
    let x_141 : i32 = data[x_139];
    temp[x_138] = x_141;

    continuing {
      let x_143 : i32 = j_1;
      j_1 = (x_143 + 1);
    }
  }
  mergeSort_();
  let x_147 : f32 = gl_FragCoord.y;
  if ((i32(x_147) < 30)) {
    let x_154 : i32 = data[0];
    grey = (0.5 + (f32(x_154) / 10.0));
  } else {
    let x_159 : f32 = gl_FragCoord.y;
    if ((i32(x_159) < 60)) {
      let x_166 : i32 = data[1];
      grey = (0.5 + (f32(x_166) / 10.0));
    } else {
      let x_171 : f32 = gl_FragCoord.y;
      if ((i32(x_171) < 90)) {
        let x_178 : i32 = data[2];
        grey = (0.5 + (f32(x_178) / 10.0));
      } else {
        let x_183 : f32 = gl_FragCoord.y;
        if ((i32(x_183) < 120)) {
          let x_190 : i32 = data[3];
          grey = (0.5 + (f32(x_190) / 10.0));
        } else {
          let x_195 : f32 = gl_FragCoord.y;
          if ((i32(x_195) < 150)) {
            discard;
          } else {
            let x_202 : f32 = gl_FragCoord.y;
            if ((i32(x_202) < 180)) {
              let x_209 : i32 = data[5];
              grey = (0.5 + (f32(x_209) / 10.0));
            } else {
              let x_214 : f32 = gl_FragCoord.y;
              if ((i32(x_214) < 210)) {
                let x_221 : i32 = data[6];
                grey = (0.5 + (f32(x_221) / 10.0));
              } else {
                let x_226 : f32 = gl_FragCoord.y;
                if ((i32(x_226) < 240)) {
                  let x_233 : i32 = data[7];
                  grey = (0.5 + (f32(x_233) / 10.0));
                } else {
                  let x_238 : f32 = gl_FragCoord.y;
                  if ((i32(x_238) < 270)) {
                    let x_245 : i32 = data[8];
                    grey = (0.5 + (f32(x_245) / 10.0));
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
  let x_249 : f32 = grey;
  let x_250 : vec3<f32> = vec3<f32>(x_249, x_249, x_249);
  x_GLF_color = vec4<f32>(x_250.x, x_250.y, x_250.z, 1.0);
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
