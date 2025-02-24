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
  let x_303 : i32 = *(f);
  k = x_303;
  let x_304 : i32 = *(f);
  i = x_304;
  let x_305 : i32 = *(mid);
  j = (x_305 + 1);
  loop {
    let x_311 : i32 = i;
    let x_312 : i32 = *(mid);
    let x_314 : i32 = j;
    let x_315 : i32 = *(to);
    if (((x_311 <= x_312) & (x_314 <= x_315))) {
    } else {
      break;
    }
    let x_319 : i32 = i;
    let x_321 : i32 = data[x_319];
    let x_322 : i32 = j;
    let x_324 : i32 = data[x_322];
    if ((x_321 < x_324)) {
      let x_329 : i32 = k;
      k = (x_329 + 1);
      let x_331 : i32 = i;
      i = (x_331 + 1);
      let x_334 : i32 = data[x_331];
      temp[x_329] = x_334;
    } else {
      let x_336 : i32 = k;
      k = (x_336 + 1);
      let x_338 : i32 = j;
      j = (x_338 + 1);
      let x_341 : i32 = data[x_338];
      temp[x_336] = x_341;
    }
  }
  loop {
    let x_347 : i32 = i;
    let x_349 : i32 = i;
    let x_350 : i32 = *(mid);
    if (((x_347 < 10) & (x_349 <= x_350))) {
    } else {
      break;
    }
    let x_354 : i32 = k;
    k = (x_354 + 1);
    let x_356 : i32 = i;
    i = (x_356 + 1);
    let x_359 : i32 = data[x_356];
    temp[x_354] = x_359;
  }
  let x_361 : i32 = *(f);
  i_1 = x_361;
  loop {
    let x_366 : i32 = i_1;
    let x_367 : i32 = *(to);
    if ((x_366 <= x_367)) {
    } else {
      break;
    }
    let x_370 : i32 = i_1;
    let x_371 : i32 = i_1;
    let x_373 : i32 = temp[x_371];
    data[x_370] = x_373;

    continuing {
      let x_375 : i32 = i_1;
      i_1 = (x_375 + 1);
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
      let x_173 : array<i32, 10u> = data;
      data = array<i32, 10u>(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      data = x_173;
      x_89 = ((x_170 + x_171) - 1);
      let x_175 : i32 = x_91;
      let x_176 : i32 = x_92;
      let x_180 : i32 = x_93;
      x_88 = min(((x_175 + (2 * x_176)) - 1), x_180);
      let x_182 : i32 = x_90;
      x_87 = x_182;
      let x_183 : i32 = x_89;
      x_86 = x_183;
      let x_184 : i32 = x_88;
      x_85 = x_184;
      merge_i1_i1_i1_(&(x_87), &(x_86), &(x_85));

      continuing {
        let x_186 : i32 = x_92;
        let x_188 : i32 = x_91;
        x_91 = (x_188 + (2 * x_186));
      }
    }

    continuing {
      let x_190 : i32 = x_92;
      x_92 = (2 * x_190);
    }
  }
  let x_194 : f32 = gl_FragCoord.y;
  if ((i32(x_194) < 30)) {
    let x_201 : i32 = data[0];
    grey = (0.5 + (f32(x_201) / 10.0));
  } else {
    let x_206 : f32 = gl_FragCoord.y;
    if ((i32(x_206) < 60)) {
      let x_213 : i32 = data[1];
      grey = (0.5 + (f32(x_213) / 10.0));
    } else {
      let x_218 : f32 = gl_FragCoord.y;
      if ((i32(x_218) < 90)) {
        let x_225 : i32 = data[2];
        grey = (0.5 + (f32(x_225) / 10.0));
      } else {
        let x_230 : f32 = gl_FragCoord.y;
        if ((i32(x_230) < 120)) {
          let x_237 : i32 = data[3];
          grey = (0.5 + (f32(x_237) / 10.0));
        } else {
          let x_242 : f32 = gl_FragCoord.y;
          if ((i32(x_242) < 150)) {
            discard;
          } else {
            let x_249 : f32 = gl_FragCoord.y;
            if ((i32(x_249) < 180)) {
              let x_256 : i32 = data[5];
              grey = (0.5 + (f32(x_256) / 10.0));
            } else {
              let x_261 : f32 = gl_FragCoord.y;
              if ((i32(x_261) < 210)) {
                let x_268 : i32 = data[6];
                grey = (0.5 + (f32(x_268) / 10.0));
              } else {
                let x_273 : f32 = gl_FragCoord.y;
                if ((i32(x_273) < 240)) {
                  let x_280 : i32 = data[7];
                  grey = (0.5 + (f32(x_280) / 10.0));
                } else {
                  let x_285 : f32 = gl_FragCoord.y;
                  if ((i32(x_285) < 270)) {
                    let x_292 : i32 = data[8];
                    grey = (0.5 + (f32(x_292) / 10.0));
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
  let x_296 : f32 = grey;
  let x_297 : vec3<f32> = vec3<f32>(x_296, x_296, x_296);
  x_GLF_color = vec4<f32>(x_297.x, x_297.y, x_297.z, 1.0);
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
    let x_382 : i32 = m;
    let x_383 : i32 = high;
    if ((x_382 <= x_383)) {
    } else {
      break;
    }
    let x_386 : i32 = low;
    i_2 = x_386;
    loop {
      let x_391 : i32 = i_2;
      let x_392 : i32 = high;
      if ((x_391 < x_392)) {
      } else {
        break;
      }
      let x_395 : i32 = i_2;
      f_1 = x_395;
      let x_396 : i32 = i_2;
      let x_397 : i32 = m;
      mid_1 = ((x_396 + x_397) - 1);
      let x_400 : i32 = i_2;
      let x_401 : i32 = m;
      let x_405 : i32 = high;
      to_1 = min(((x_400 + (2 * x_401)) - 1), x_405);
      let x_407 : i32 = f_1;
      param = x_407;
      let x_408 : i32 = mid_1;
      param_1 = x_408;
      let x_409 : i32 = to_1;
      param_2 = x_409;
      merge_i1_i1_i1_(&(param), &(param_1), &(param_2));

      continuing {
        let x_411 : i32 = m;
        let x_413 : i32 = i_2;
        i_2 = (x_413 + (2 * x_411));
      }
    }

    continuing {
      let x_415 : i32 = m;
      m = (2 * x_415);
    }
  }
  return;
}
