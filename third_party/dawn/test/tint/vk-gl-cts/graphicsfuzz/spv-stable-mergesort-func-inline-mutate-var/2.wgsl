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
  let x_302 : i32 = *(f);
  k = x_302;
  let x_303 : i32 = *(f);
  i = x_303;
  let x_304 : i32 = *(mid);
  j = (x_304 + 1);
  loop {
    let x_310 : i32 = i;
    let x_311 : i32 = *(mid);
    let x_313 : i32 = j;
    let x_314 : i32 = *(to);
    if (((x_310 <= x_311) & (x_313 <= x_314))) {
    } else {
      break;
    }
    let x_318 : i32 = i;
    let x_320 : i32 = data[x_318];
    let x_321 : i32 = j;
    let x_323 : i32 = data[x_321];
    if ((x_320 < x_323)) {
      let x_328 : i32 = k;
      k = (x_328 + 1);
      let x_330 : i32 = i;
      i = (x_330 + 1);
      let x_333 : i32 = data[x_330];
      temp[x_328] = x_333;
    } else {
      let x_335 : i32 = k;
      k = (x_335 + 1);
      let x_337 : i32 = j;
      j = (x_337 + 1);
      let x_340 : i32 = data[x_337];
      temp[x_335] = x_340;
    }
  }
  loop {
    let x_346 : i32 = i;
    let x_348 : i32 = i;
    let x_349 : i32 = *(mid);
    if (((x_346 < 10) & (x_348 <= x_349))) {
    } else {
      break;
    }
    let x_353 : i32 = k;
    k = (x_353 + 1);
    let x_355 : i32 = i;
    i = (x_355 + 1);
    let x_358 : i32 = data[x_355];
    temp[x_353] = x_358;
  }
  let x_360 : i32 = *(f);
  i_1 = x_360;
  loop {
    let x_365 : i32 = i_1;
    let x_366 : i32 = *(to);
    if ((x_365 <= x_366)) {
    } else {
      break;
    }
    let x_369 : i32 = i_1;
    let x_370 : i32 = i_1;
    let x_372 : i32 = temp[x_370];
    data[x_369] = x_372;

    continuing {
      let x_374 : i32 = i_1;
      i_1 = (x_374 + 1);
    }
  }
  return;
}

fn main_1() {
  var x_85 : i32;
  var x_86 : i32;
  var x_87 : i32;
  var x_88 : i32;
  var x_89 : i32;
  var x_90 : i32;
  var x_91 : i32;
  var x_92 : i32;
  var x_93 : i32;
  var x_94 : i32;
  var i_3 : i32;
  var j_1 : i32;
  var grey : f32;
  let x_96 : f32 = x_28.injectionSwitch.x;
  i_3 = i32(x_96);
  loop {
    let x_102 : i32 = i_3;
    switch(x_102) {
      case 9: {
        let x_132 : i32 = i_3;
        data[x_132] = -5;
      }
      case 8: {
        let x_130 : i32 = i_3;
        data[x_130] = -4;
      }
      case 7: {
        let x_128 : i32 = i_3;
        data[x_128] = -3;
      }
      case 6: {
        let x_126 : i32 = i_3;
        data[x_126] = -2;
      }
      case 5: {
        let x_124 : i32 = i_3;
        data[x_124] = -1;
      }
      case 4: {
        let x_122 : i32 = i_3;
        data[x_122] = 0;
      }
      case 3: {
        let x_120 : i32 = i_3;
        data[x_120] = 1;
      }
      case 2: {
        let x_118 : i32 = i_3;
        data[x_118] = 2;
      }
      case 1: {
        let x_116 : i32 = i_3;
        data[x_116] = 3;
      }
      case 0: {
        let x_114 : i32 = i_3;
        data[x_114] = 4;
      }
      default: {
      }
    }
    let x_134 : i32 = i_3;
    i_3 = (x_134 + 1);

    continuing {
      let x_136 : i32 = i_3;
      break if !(x_136 < 10);
    }
  }
  j_1 = 0;
  loop {
    let x_142 : i32 = j_1;
    if ((x_142 < 10)) {
    } else {
      break;
    }
    let x_145 : i32 = j_1;
    let x_146 : i32 = j_1;
    let x_148 : i32 = data[x_146];
    temp[x_145] = x_148;

    continuing {
      let x_150 : i32 = j_1;
      j_1 = (x_150 + 1);
    }
  }
  x_94 = 0;
  x_93 = 9;
  x_92 = 1;
  loop {
    let x_156 : i32 = x_92;
    let x_157 : i32 = x_93;
    if ((x_156 <= x_157)) {
    } else {
      break;
    }
    let x_160 : i32 = x_94;
    x_91 = x_160;
    loop {
      let x_165 : i32 = x_91;
      let x_166 : i32 = x_93;
      if ((x_165 < x_166)) {
      } else {
        break;
      }
      let x_169 : i32 = x_91;
      x_90 = x_169;
      let x_170 : i32 = x_91;
      let x_171 : i32 = x_92;
      x_89 = ((x_170 + x_171) - 1);
      let x_174 : i32 = x_91;
      let x_175 : i32 = x_92;
      let x_179 : i32 = x_93;
      x_88 = min(((x_174 + (2 * x_175)) - 1), x_179);
      let x_181 : i32 = x_90;
      x_87 = x_181;
      let x_182 : i32 = x_89;
      x_86 = x_182;
      let x_183 : i32 = x_88;
      x_85 = x_183;
      merge_i1_i1_i1_(&(x_87), &(x_86), &(x_85));

      continuing {
        let x_185 : i32 = x_92;
        let x_187 : i32 = x_91;
        x_91 = (x_187 + (2 * x_185));
      }
    }

    continuing {
      let x_189 : i32 = x_92;
      x_92 = (2 * x_189);
    }
  }
  let x_193 : f32 = gl_FragCoord.y;
  if ((i32(x_193) < 30)) {
    let x_200 : i32 = data[0];
    grey = (0.5 + (f32(x_200) / 10.0));
  } else {
    let x_205 : f32 = gl_FragCoord.y;
    if ((i32(x_205) < 60)) {
      let x_212 : i32 = data[1];
      grey = (0.5 + (f32(x_212) / 10.0));
    } else {
      let x_217 : f32 = gl_FragCoord.y;
      if ((i32(x_217) < 90)) {
        let x_224 : i32 = data[2];
        grey = (0.5 + (f32(x_224) / 10.0));
      } else {
        let x_229 : f32 = gl_FragCoord.y;
        if ((i32(x_229) < 120)) {
          let x_236 : i32 = data[3];
          grey = (0.5 + (f32(x_236) / 10.0));
        } else {
          let x_241 : f32 = gl_FragCoord.y;
          if ((i32(x_241) < 150)) {
            discard;
          } else {
            let x_248 : f32 = gl_FragCoord.y;
            if ((i32(x_248) < 180)) {
              let x_255 : i32 = data[5];
              grey = (0.5 + (f32(x_255) / 10.0));
            } else {
              let x_260 : f32 = gl_FragCoord.y;
              if ((i32(x_260) < 210)) {
                let x_267 : i32 = data[6];
                grey = (0.5 + (f32(x_267) / 10.0));
              } else {
                let x_272 : f32 = gl_FragCoord.y;
                if ((i32(x_272) < 240)) {
                  let x_279 : i32 = data[7];
                  grey = (0.5 + (f32(x_279) / 10.0));
                } else {
                  let x_284 : f32 = gl_FragCoord.y;
                  if ((i32(x_284) < 270)) {
                    let x_291 : i32 = data[8];
                    grey = (0.5 + (f32(x_291) / 10.0));
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
  let x_295 : f32 = grey;
  let x_296 : vec3<f32> = vec3<f32>(x_295, x_295, x_295);
  x_GLF_color = vec4<f32>(x_296.x, x_296.y, x_296.z, 1.0);
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
    let x_381 : i32 = m;
    let x_382 : i32 = high;
    if ((x_381 <= x_382)) {
    } else {
      break;
    }
    let x_385 : i32 = low;
    i_2 = x_385;
    loop {
      let x_390 : i32 = i_2;
      let x_391 : i32 = high;
      if ((x_390 < x_391)) {
      } else {
        break;
      }
      let x_394 : i32 = i_2;
      f_1 = x_394;
      let x_395 : i32 = i_2;
      let x_396 : i32 = m;
      mid_1 = ((x_395 + x_396) - 1);
      let x_399 : i32 = i_2;
      let x_400 : i32 = m;
      let x_404 : i32 = high;
      to_1 = min(((x_399 + (2 * x_400)) - 1), x_404);
      let x_406 : i32 = f_1;
      param = x_406;
      let x_407 : i32 = mid_1;
      param_1 = x_407;
      let x_408 : i32 = to_1;
      param_2 = x_408;
      merge_i1_i1_i1_(&(param), &(param_1), &(param_2));

      continuing {
        let x_410 : i32 = m;
        let x_412 : i32 = i_2;
        i_2 = (x_412 + (2 * x_410));
      }
    }

    continuing {
      let x_414 : i32 = m;
      m = (2 * x_414);
    }
  }
  return;
}
