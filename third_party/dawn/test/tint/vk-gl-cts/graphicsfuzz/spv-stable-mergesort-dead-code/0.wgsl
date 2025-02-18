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
  let x_251 : i32 = *(f);
  k = x_251;
  let x_252 : i32 = *(f);
  i = x_252;
  let x_253 : i32 = *(mid);
  j = (x_253 + 1);
  loop {
    let x_259 : i32 = i;
    let x_260 : i32 = *(mid);
    let x_262 : i32 = j;
    let x_263 : i32 = *(to);
    if (((x_259 <= x_260) & (x_262 <= x_263))) {
    } else {
      break;
    }
    let x_267 : i32 = i;
    let x_269 : i32 = data[x_267];
    let x_270 : i32 = j;
    let x_272 : i32 = data[x_270];
    if ((x_269 < x_272)) {
      let x_277 : i32 = k;
      k = (x_277 + 1);
      let x_279 : i32 = i;
      i = (x_279 + 1);
      let x_282 : i32 = data[x_279];
      temp[x_277] = x_282;
    } else {
      let x_284 : i32 = k;
      k = (x_284 + 1);
      let x_286 : i32 = j;
      j = (x_286 + 1);
      let x_289 : i32 = data[x_286];
      temp[x_284] = x_289;
    }
  }
  loop {
    let x_295 : i32 = i;
    let x_297 : i32 = i;
    let x_298 : i32 = *(mid);
    if (((x_295 < 10) & (x_297 <= x_298))) {
    } else {
      break;
    }
    let x_302 : i32 = k;
    k = (x_302 + 1);
    let x_304 : i32 = i;
    i = (x_304 + 1);
    let x_307 : i32 = data[x_304];
    temp[x_302] = x_307;
  }
  let x_309 : i32 = *(f);
  i_1 = x_309;
  loop {
    let x_314 : i32 = i_1;
    let x_315 : i32 = *(to);
    if ((x_314 <= x_315)) {
    } else {
      break;
    }
    let x_318 : i32 = i_1;
    let x_319 : i32 = i_1;
    let x_321 : i32 = temp[x_319];
    data[x_318] = x_321;

    continuing {
      let x_323 : i32 = i_1;
      i_1 = (x_323 + 1);
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
    let x_330 : i32 = m;
    let x_331 : i32 = high;
    if ((x_330 <= x_331)) {
    } else {
      break;
    }
    let x_334 : i32 = low;
    i_2 = x_334;
    loop {
      let x_339 : i32 = i_2;
      let x_340 : i32 = high;
      if ((x_339 < x_340)) {
      } else {
        break;
      }
      let x_343 : i32 = i_2;
      f_1 = x_343;
      let x_344 : i32 = i_2;
      let x_345 : i32 = m;
      mid_1 = ((x_344 + x_345) - 1);
      let x_348 : i32 = i_2;
      let x_349 : i32 = m;
      let x_353 : i32 = high;
      to_1 = min(((x_348 + (2 * x_349)) - 1), x_353);
      let x_355 : i32 = f_1;
      param = x_355;
      let x_356 : i32 = mid_1;
      param_1 = x_356;
      let x_357 : i32 = to_1;
      param_2 = x_357;
      merge_i1_i1_i1_(&(param), &(param_1), &(param_2));

      continuing {
        let x_359 : i32 = m;
        let x_361 : i32 = i_2;
        i_2 = (x_361 + (2 * x_359));
      }
    }

    continuing {
      let x_363 : i32 = m;
      m = (2 * x_363);
    }
  }
  return;
}

fn main_1() {
  var i_3 : i32;
  var j_1 : i32;
  var grey : f32;
  let x_84 : f32 = x_28.injectionSwitch.x;
  i_3 = i32(x_84);
  loop {
    let x_90 : i32 = i_3;
    switch(x_90) {
      case 9: {
        let x_120 : i32 = i_3;
        data[x_120] = -5;
      }
      case 8: {
        let x_118 : i32 = i_3;
        data[x_118] = -4;
      }
      case 7: {
        let x_116 : i32 = i_3;
        data[x_116] = -3;
      }
      case 6: {
        let x_114 : i32 = i_3;
        data[x_114] = -2;
      }
      case 5: {
        let x_112 : i32 = i_3;
        data[x_112] = -1;
      }
      case 4: {
        let x_110 : i32 = i_3;
        data[x_110] = 0;
      }
      case 3: {
        let x_108 : i32 = i_3;
        data[x_108] = 1;
      }
      case 2: {
        let x_106 : i32 = i_3;
        data[x_106] = 2;
      }
      case 1: {
        let x_104 : i32 = i_3;
        data[x_104] = 3;
      }
      case 0: {
        let x_102 : i32 = i_3;
        data[x_102] = 4;
      }
      default: {
      }
    }
    let x_122 : i32 = i_3;
    i_3 = (x_122 + 1);

    continuing {
      let x_124 : i32 = i_3;
      break if !(x_124 < 10);
    }
  }
  j_1 = 0;
  loop {
    let x_130 : i32 = j_1;
    if ((x_130 < 10)) {
    } else {
      break;
    }
    let x_133 : i32 = j_1;
    let x_134 : i32 = j_1;
    let x_136 : i32 = data[x_134];
    temp[x_133] = x_136;

    continuing {
      let x_138 : i32 = j_1;
      j_1 = (x_138 + 1);
    }
  }
  mergeSort_();
  let x_142 : f32 = gl_FragCoord.y;
  if ((i32(x_142) < 30)) {
    let x_149 : i32 = data[0];
    grey = (0.5 + (f32(x_149) / 10.0));
  } else {
    let x_154 : f32 = gl_FragCoord.y;
    if ((i32(x_154) < 60)) {
      let x_161 : i32 = data[1];
      grey = (0.5 + (f32(x_161) / 10.0));
    } else {
      let x_166 : f32 = gl_FragCoord.y;
      if ((i32(x_166) < 90)) {
        let x_173 : i32 = data[2];
        grey = (0.5 + (f32(x_173) / 10.0));
      } else {
        let x_178 : f32 = gl_FragCoord.y;
        if ((i32(x_178) < 120)) {
          let x_185 : i32 = data[3];
          grey = (0.5 + (f32(x_185) / 10.0));
        } else {
          let x_190 : f32 = gl_FragCoord.y;
          if ((i32(x_190) < 150)) {
            discard;
          } else {
            let x_197 : f32 = gl_FragCoord.y;
            if ((i32(x_197) < 180)) {
              let x_204 : i32 = data[5];
              grey = (0.5 + (f32(x_204) / 10.0));
            } else {
              let x_209 : f32 = gl_FragCoord.y;
              if ((i32(x_209) < 210)) {
                let x_216 : i32 = data[6];
                grey = (0.5 + (f32(x_216) / 10.0));
              } else {
                let x_221 : f32 = gl_FragCoord.y;
                if ((i32(x_221) < 240)) {
                  let x_228 : i32 = data[7];
                  grey = (0.5 + (f32(x_228) / 10.0));
                } else {
                  let x_233 : f32 = gl_FragCoord.y;
                  if ((i32(x_233) < 270)) {
                    let x_240 : i32 = data[8];
                    grey = (0.5 + (f32(x_240) / 10.0));
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
  let x_244 : f32 = grey;
  let x_245 : vec3<f32> = vec3<f32>(x_244, x_244, x_244);
  x_GLF_color = vec4<f32>(x_245.x, x_245.y, x_245.z, 1.0);
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
