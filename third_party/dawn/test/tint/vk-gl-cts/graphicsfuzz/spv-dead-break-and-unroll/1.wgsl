struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> map : array<i32, 256u>;

var<private> x_GLF_color : vec4<f32>;

var<private> x_60 : mat2x4<f32> = mat2x4<f32>(vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0));

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
  let x_63 : vec4<f32> = gl_FragCoord;
  let x_67 : vec2<f32> = x_7.resolution;
  let x_68 : i32 = -((256 - 14));
  pos = (vec2<f32>(x_63.x, x_63.y) / x_67);
  let x_71 : f32 = pos.x;
  let x_75 : f32 = pos.y;
  ipos = vec2<i32>(i32((x_71 * 16.0)), i32((x_75 * 16.0)));
  i = 0;
  loop {
    let x_83 : i32 = i;
    if ((x_83 < 256)) {
    } else {
      break;
    }
    let x_86 : i32 = i;
    map[x_86] = 0;

    continuing {
      let x_88 : i32 = i;
      i = (x_88 + 1);
    }
  }
  p = vec2<i32>(0, 0);
  canwalk = true;
  v = 0;
  loop {
    var x_110 : bool;
    var x_130 : bool;
    var x_150 : bool;
    var x_171 : bool;
    var x_111_phi : bool;
    var x_131_phi : bool;
    var x_151_phi : bool;
    var x_172_phi : bool;
    let x_94 : i32 = v;
    v = (x_94 + 1);
    directions = 0;
    let x_97 : i32 = p.x;
    let x_98 : bool = (x_97 > 0);
    x_111_phi = x_98;
    if (x_98) {
      let x_102 : i32 = p.x;
      let x_105 : i32 = p.y;
      let x_109 : i32 = map[((x_102 - 2) + (x_105 * 16))];
      x_110 = (x_109 == 0);
      x_111_phi = x_110;
    }
    let x_111 : bool = x_111_phi;
    if (x_111) {
      let x_114 : i32 = directions;
      directions = (x_114 + 1);
    }
    let x_117 : i32 = p.y;
    let x_118 : bool = (x_117 > 0);
    x_131_phi = x_118;
    if (x_118) {
      let x_122 : i32 = p.x;
      let x_124 : i32 = p.y;
      let x_129 : i32 = map[(x_122 + ((x_124 - 2) * 16))];
      x_130 = (x_129 == 0);
      x_131_phi = x_130;
    }
    let x_131 : bool = x_131_phi;
    if (x_131) {
      let x_134 : i32 = directions;
      directions = (x_134 + 1);
    }
    let x_137 : i32 = p.x;
    let x_138 : bool = (x_137 < 14);
    x_151_phi = x_138;
    if (x_138) {
      let x_142 : i32 = p.x;
      let x_145 : i32 = p.y;
      let x_149 : i32 = map[((x_142 + 2) + (x_145 * 16))];
      x_150 = (x_149 == 0);
      x_151_phi = x_150;
    }
    let x_151 : bool = x_151_phi;
    if (x_151) {
      let x_154 : i32 = directions;
      directions = (x_154 + 1);
    }
    let x_156 : i32 = (256 - x_68);
    let x_158 : i32 = p.y;
    let x_159 : bool = (x_158 < 14);
    x_172_phi = x_159;
    if (x_159) {
      let x_163 : i32 = p.x;
      let x_165 : i32 = p.y;
      let x_170 : i32 = map[(x_163 + ((x_165 + 2) * 16))];
      x_171 = (x_170 == 0);
      x_172_phi = x_171;
    }
    let x_172 : bool = x_172_phi;
    if (x_172) {
      let x_175 : i32 = directions;
      directions = (x_175 + 1);
    }
    var x_237 : bool;
    var x_250 : bool;
    var x_289 : bool;
    var x_302 : bool;
    var x_341 : bool;
    var x_354 : bool;
    var x_393 : bool;
    var x_406 : bool;
    var x_238_phi : bool;
    var x_251_phi : bool;
    var x_290_phi : bool;
    var x_303_phi : bool;
    var x_342_phi : bool;
    var x_355_phi : bool;
    var x_394_phi : bool;
    var x_407_phi : bool;
    let x_177 : i32 = directions;
    if ((x_177 == 0)) {
      canwalk = false;
      i = 0;
      loop {
        let x_186 : i32 = i;
        if ((x_186 < 8)) {
        } else {
          break;
        }
        j = 0;
        let x_189 : i32 = (x_156 - x_186);
        x_60 = mat2x4<f32>(vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0));
        if (false) {
          continue;
        }
        loop {
          let x_194 : i32 = j;
          if ((x_194 < 8)) {
          } else {
            break;
          }
          let x_197 : i32 = j;
          let x_199 : i32 = i;
          let x_204 : i32 = map[((x_197 * 2) + ((x_199 * 2) * 16))];
          if ((x_204 == 0)) {
            let x_208 : i32 = j;
            p.x = (x_208 * 2);
            let x_211 : i32 = i;
            p.y = (x_211 * 2);
            canwalk = true;
          }

          continuing {
            let x_214 : i32 = j;
            j = (x_214 + 1);
          }
        }

        continuing {
          let x_216 : i32 = i;
          i = (x_216 + 1);
        }
      }
      let x_219 : i32 = p.x;
      let x_221 : i32 = p.y;
      map[(x_219 + (x_221 * 16))] = 1;
    } else {
      let x_225 : i32 = v;
      let x_226 : i32 = directions;
      d = (x_225 % x_226);
      let x_228 : i32 = directions;
      let x_229 : i32 = v;
      v = (x_229 + x_228);
      let x_231 : i32 = d;
      let x_232 : bool = (x_231 >= 0);
      x_238_phi = x_232;
      if (x_232) {
        let x_236 : i32 = p.x;
        x_237 = (x_236 > 0);
        x_238_phi = x_237;
      }
      let x_238 : bool = x_238_phi;
      x_251_phi = x_238;
      if (x_238) {
        let x_242 : i32 = p.x;
        let x_245 : i32 = p.y;
        let x_249 : i32 = map[((x_242 - 2) + (x_245 * 16))];
        x_250 = (x_249 == 0);
        x_251_phi = x_250;
      }
      let x_251 : bool = x_251_phi;
      if (x_251) {
        let x_254 : i32 = d;
        d = (x_254 - 1);
        let x_257 : i32 = p.x;
        let x_259 : i32 = p.y;
        map[(x_257 + (x_259 * 16))] = 1;
        let x_264 : i32 = p.x;
        let x_267 : i32 = p.y;
        map[((x_264 - 1) + (x_267 * 16))] = 1;
        let x_272 : i32 = p.x;
        let x_275 : i32 = p.y;
        map[((x_272 - 2) + (x_275 * 16))] = 1;
        let x_280 : i32 = p.x;
        p.x = (x_280 - 2);
      }
      let x_283 : i32 = d;
      let x_284 : bool = (x_283 >= 0);
      x_290_phi = x_284;
      if (x_284) {
        let x_288 : i32 = p.y;
        x_289 = (x_288 > 0);
        x_290_phi = x_289;
      }
      let x_290 : bool = x_290_phi;
      x_303_phi = x_290;
      if (x_290) {
        let x_294 : i32 = p.x;
        let x_296 : i32 = p.y;
        let x_301 : i32 = map[(x_294 + ((x_296 - 2) * 16))];
        x_302 = (x_301 == 0);
        x_303_phi = x_302;
      }
      let x_303 : bool = x_303_phi;
      if (x_303) {
        let x_306 : i32 = d;
        d = (x_306 - 1);
        let x_309 : i32 = p.x;
        let x_311 : i32 = p.y;
        map[(x_309 + (x_311 * 16))] = 1;
        let x_316 : i32 = p.x;
        let x_318 : i32 = p.y;
        map[(x_316 + ((x_318 - 1) * 16))] = 1;
        let x_324 : i32 = p.x;
        let x_326 : i32 = p.y;
        map[(x_324 + ((x_326 - 2) * 16))] = 1;
        let x_332 : i32 = p.y;
        p.y = (x_332 - 2);
      }
      let x_335 : i32 = d;
      let x_336 : bool = (x_335 >= 0);
      x_342_phi = x_336;
      if (x_336) {
        let x_340 : i32 = p.x;
        x_341 = (x_340 < 14);
        x_342_phi = x_341;
      }
      let x_342 : bool = x_342_phi;
      x_355_phi = x_342;
      if (x_342) {
        let x_346 : i32 = p.x;
        let x_349 : i32 = p.y;
        let x_353 : i32 = map[((x_346 + 2) + (x_349 * 16))];
        x_354 = (x_353 == 0);
        x_355_phi = x_354;
      }
      let x_355 : bool = x_355_phi;
      if (x_355) {
        let x_358 : i32 = d;
        d = (x_358 - 1);
        let x_361 : i32 = p.x;
        let x_363 : i32 = p.y;
        map[(x_361 + (x_363 * 16))] = 1;
        let x_368 : i32 = p.x;
        let x_371 : i32 = p.y;
        map[((x_368 + 1) + (x_371 * 16))] = 1;
        let x_376 : i32 = p.x;
        let x_379 : i32 = p.y;
        map[((x_376 + 2) + (x_379 * 16))] = 1;
        let x_384 : i32 = p.x;
        p.x = (x_384 + 2);
      }
      let x_387 : i32 = d;
      let x_388 : bool = (x_387 >= 0);
      x_394_phi = x_388;
      if (x_388) {
        let x_392 : i32 = p.y;
        x_393 = (x_392 < 14);
        x_394_phi = x_393;
      }
      let x_394 : bool = x_394_phi;
      x_407_phi = x_394;
      if (x_394) {
        let x_398 : i32 = p.x;
        let x_400 : i32 = p.y;
        let x_405 : i32 = map[(x_398 + ((x_400 + 2) * 16))];
        x_406 = (x_405 == 0);
        x_407_phi = x_406;
      }
      let x_407 : bool = x_407_phi;
      if (x_407) {
        let x_410 : i32 = d;
        d = (x_410 - 1);
        let x_413 : i32 = p.x;
        let x_415 : i32 = p.y;
        map[(x_413 + (x_415 * 16))] = 1;
        let x_420 : i32 = p.x;
        let x_422 : i32 = p.y;
        map[(x_420 + ((x_422 + 1) * 16))] = 1;
        let x_428 : i32 = p.x;
        let x_430 : i32 = p.y;
        map[(x_428 + ((x_430 + 2) * 16))] = 1;
        let x_436 : i32 = p.y;
        p.y = (x_436 + 2);
      }
    }
    let x_440 : i32 = ipos.y;
    let x_443 : i32 = ipos.x;
    let x_446 : i32 = map[((x_440 * 16) + x_443)];
    if ((x_446 == 1)) {
      x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
      return;
    }

    continuing {
      let x_450 : bool = canwalk;
      break if !(x_450);
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
