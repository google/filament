struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> map : array<i32, 256u>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var pos : vec2<f32>;
  var ipos : vec2<i32>;
  var i : i32;
  var p : vec2<i32>;
  var canwalk : bool;
  var v : i32;
  var directions : i32;
  var j : i32;
  var d : i32;
  let x_57 : vec4<f32> = gl_FragCoord;
  let x_60 : vec2<f32> = x_7.resolution;
  pos = (vec2<f32>(x_57.x, x_57.y) / x_60);
  let x_63 : f32 = pos.x;
  let x_67 : f32 = pos.y;
  ipos = vec2<i32>(i32((x_63 * 16.0)), i32((x_67 * 16.0)));
  i = 0;
  loop {
    let x_75 : i32 = i;
    if ((x_75 < 256)) {
    } else {
      break;
    }
    let x_78 : i32 = i;
    map[x_78] = 0;

    continuing {
      let x_80 : i32 = i;
      i = (x_80 + 1);
    }
  }
  p = vec2<i32>(0, 0);
  canwalk = true;
  v = 0;
  loop {
    var x_102 : bool;
    var x_122 : bool;
    var x_142 : bool;
    var x_162 : bool;
    var x_103_phi : bool;
    var x_123_phi : bool;
    var x_143_phi : bool;
    var x_163_phi : bool;
    let x_86 : i32 = v;
    v = (x_86 + 1);
    directions = 0;
    let x_89 : i32 = p.x;
    let x_90 : bool = (x_89 > 0);
    x_103_phi = x_90;
    if (x_90) {
      let x_94 : i32 = p.x;
      let x_97 : i32 = p.y;
      let x_101 : i32 = map[((x_94 - 2) + (x_97 * 16))];
      x_102 = (x_101 == 0);
      x_103_phi = x_102;
    }
    let x_103 : bool = x_103_phi;
    if (x_103) {
      let x_106 : i32 = directions;
      directions = (x_106 + 1);
    }
    let x_109 : i32 = p.y;
    let x_110 : bool = (x_109 > 0);
    x_123_phi = x_110;
    if (x_110) {
      let x_114 : i32 = p.x;
      let x_116 : i32 = p.y;
      let x_121 : i32 = map[(x_114 + ((x_116 - 2) * 16))];
      x_122 = (x_121 == 0);
      x_123_phi = x_122;
    }
    let x_123 : bool = x_123_phi;
    if (x_123) {
      let x_126 : i32 = directions;
      directions = (x_126 + 1);
    }
    let x_129 : i32 = p.x;
    let x_130 : bool = (x_129 < 14);
    x_143_phi = x_130;
    if (x_130) {
      let x_134 : i32 = p.x;
      let x_137 : i32 = p.y;
      let x_141 : i32 = map[((x_134 + 2) + (x_137 * 16))];
      x_142 = (x_141 == 0);
      x_143_phi = x_142;
    }
    let x_143 : bool = x_143_phi;
    if (x_143) {
      let x_146 : i32 = directions;
      directions = (x_146 + 1);
    }
    let x_149 : i32 = p.y;
    let x_150 : bool = (x_149 < 14);
    x_163_phi = x_150;
    if (x_150) {
      let x_154 : i32 = p.x;
      let x_156 : i32 = p.y;
      let x_161 : i32 = map[(x_154 + ((x_156 + 2) * 16))];
      x_162 = (x_161 == 0);
      x_163_phi = x_162;
    }
    let x_163 : bool = x_163_phi;
    if (x_163) {
      let x_166 : i32 = directions;
      directions = (x_166 + 1);
    }
    var x_227 : bool;
    var x_240 : bool;
    var x_279 : bool;
    var x_292 : bool;
    var x_331 : bool;
    var x_344 : bool;
    var x_383 : bool;
    var x_396 : bool;
    var x_228_phi : bool;
    var x_241_phi : bool;
    var x_280_phi : bool;
    var x_293_phi : bool;
    var x_332_phi : bool;
    var x_345_phi : bool;
    var x_384_phi : bool;
    var x_397_phi : bool;
    let x_168 : i32 = directions;
    if ((x_168 == 0)) {
      canwalk = false;
      i = 0;
      loop {
        let x_177 : i32 = i;
        if ((x_177 < 8)) {
        } else {
          break;
        }
        j = 0;
        loop {
          let x_184 : i32 = j;
          if ((x_184 < 8)) {
          } else {
            break;
          }
          let x_187 : i32 = j;
          let x_189 : i32 = i;
          let x_194 : i32 = map[((x_187 * 2) + ((x_189 * 2) * 16))];
          if ((x_194 == 0)) {
            let x_198 : i32 = j;
            p.x = (x_198 * 2);
            let x_201 : i32 = i;
            p.y = (x_201 * 2);
            canwalk = true;
          }

          continuing {
            let x_204 : i32 = j;
            j = (x_204 + 1);
          }
        }

        continuing {
          let x_206 : i32 = i;
          i = (x_206 + 1);
        }
      }
      let x_209 : i32 = p.x;
      let x_211 : i32 = p.y;
      map[(x_209 + (x_211 * 16))] = 1;
    } else {
      let x_215 : i32 = v;
      let x_216 : i32 = directions;
      d = (x_215 % x_216);
      let x_218 : i32 = directions;
      let x_219 : i32 = v;
      v = (x_219 + x_218);
      let x_221 : i32 = d;
      let x_222 : bool = (x_221 >= 0);
      x_228_phi = x_222;
      if (x_222) {
        let x_226 : i32 = p.x;
        x_227 = (x_226 > 0);
        x_228_phi = x_227;
      }
      let x_228 : bool = x_228_phi;
      x_241_phi = x_228;
      if (x_228) {
        let x_232 : i32 = p.x;
        let x_235 : i32 = p.y;
        let x_239 : i32 = map[((x_232 - 2) + (x_235 * 16))];
        x_240 = (x_239 == 0);
        x_241_phi = x_240;
      }
      let x_241 : bool = x_241_phi;
      if (x_241) {
        let x_244 : i32 = d;
        d = (x_244 - 1);
        let x_247 : i32 = p.x;
        let x_249 : i32 = p.y;
        map[(x_247 + (x_249 * 16))] = 1;
        let x_254 : i32 = p.x;
        let x_257 : i32 = p.y;
        map[((x_254 - 1) + (x_257 * 16))] = 1;
        let x_262 : i32 = p.x;
        let x_265 : i32 = p.y;
        map[((x_262 - 2) + (x_265 * 16))] = 1;
        let x_270 : i32 = p.x;
        p.x = (x_270 - 2);
      }
      let x_273 : i32 = d;
      let x_274 : bool = (x_273 >= 0);
      x_280_phi = x_274;
      if (x_274) {
        let x_278 : i32 = p.y;
        x_279 = (x_278 > 0);
        x_280_phi = x_279;
      }
      let x_280 : bool = x_280_phi;
      x_293_phi = x_280;
      if (x_280) {
        let x_284 : i32 = p.x;
        let x_286 : i32 = p.y;
        let x_291 : i32 = map[(x_284 + ((x_286 - 2) * 16))];
        x_292 = (x_291 == 0);
        x_293_phi = x_292;
      }
      let x_293 : bool = x_293_phi;
      if (x_293) {
        let x_296 : i32 = d;
        d = (x_296 - 1);
        let x_299 : i32 = p.x;
        let x_301 : i32 = p.y;
        map[(x_299 + (x_301 * 16))] = 1;
        let x_306 : i32 = p.x;
        let x_308 : i32 = p.y;
        map[(x_306 + ((x_308 - 1) * 16))] = 1;
        let x_314 : i32 = p.x;
        let x_316 : i32 = p.y;
        map[(x_314 + ((x_316 - 2) * 16))] = 1;
        let x_322 : i32 = p.y;
        p.y = (x_322 - 2);
      }
      let x_325 : i32 = d;
      let x_326 : bool = (x_325 >= 0);
      x_332_phi = x_326;
      if (x_326) {
        let x_330 : i32 = p.x;
        x_331 = (x_330 < 14);
        x_332_phi = x_331;
      }
      let x_332 : bool = x_332_phi;
      x_345_phi = x_332;
      if (x_332) {
        let x_336 : i32 = p.x;
        let x_339 : i32 = p.y;
        let x_343 : i32 = map[((x_336 + 2) + (x_339 * 16))];
        x_344 = (x_343 == 0);
        x_345_phi = x_344;
      }
      let x_345 : bool = x_345_phi;
      if (x_345) {
        let x_348 : i32 = d;
        d = (x_348 - 1);
        let x_351 : i32 = p.x;
        let x_353 : i32 = p.y;
        map[(x_351 + (x_353 * 16))] = 1;
        let x_358 : i32 = p.x;
        let x_361 : i32 = p.y;
        map[((x_358 + 1) + (x_361 * 16))] = 1;
        let x_366 : i32 = p.x;
        let x_369 : i32 = p.y;
        map[((x_366 + 2) + (x_369 * 16))] = 1;
        let x_374 : i32 = p.x;
        p.x = (x_374 + 2);
      }
      let x_377 : i32 = d;
      let x_378 : bool = (x_377 >= 0);
      x_384_phi = x_378;
      if (x_378) {
        let x_382 : i32 = p.y;
        x_383 = (x_382 < 14);
        x_384_phi = x_383;
      }
      let x_384 : bool = x_384_phi;
      x_397_phi = x_384;
      if (x_384) {
        let x_388 : i32 = p.x;
        let x_390 : i32 = p.y;
        let x_395 : i32 = map[(x_388 + ((x_390 + 2) * 16))];
        x_396 = (x_395 == 0);
        x_397_phi = x_396;
      }
      let x_397 : bool = x_397_phi;
      if (x_397) {
        let x_400 : i32 = d;
        d = (x_400 - 1);
        let x_403 : i32 = p.x;
        let x_405 : i32 = p.y;
        map[(x_403 + (x_405 * 16))] = 1;
        let x_410 : i32 = p.x;
        let x_412 : i32 = p.y;
        map[(x_410 + ((x_412 + 1) * 16))] = 1;
        let x_418 : i32 = p.x;
        let x_420 : i32 = p.y;
        map[(x_418 + ((x_420 + 2) * 16))] = 1;
        let x_426 : i32 = p.y;
        p.y = (x_426 + 2);
      }
    }
    let x_430 : i32 = ipos.y;
    let x_433 : i32 = ipos.x;
    let x_436 : i32 = map[((x_430 * 16) + x_433)];
    if ((x_436 == 1)) {
      x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
      return;
    }

    continuing {
      let x_440 : bool = canwalk;
      break if !(x_440);
    }
  }
  x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 1.0);
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
