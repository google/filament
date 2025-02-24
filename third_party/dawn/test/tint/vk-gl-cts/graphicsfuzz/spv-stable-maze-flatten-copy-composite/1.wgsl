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
  let x_59 : vec4<f32> = gl_FragCoord;
  let x_62 : vec2<f32> = x_7.resolution;
  pos = (vec2<f32>(x_59.x, x_59.y) / x_62);
  let x_65 : f32 = pos.x;
  let x_69 : f32 = pos.y;
  ipos = vec2<i32>(i32((x_65 * 16.0)), i32((x_69 * 16.0)));
  i = 0;
  loop {
    let x_77 : i32 = i;
    if ((x_77 < 256)) {
    } else {
      break;
    }
    let x_80 : i32 = i;
    map[x_80] = 0;

    continuing {
      let x_82 : i32 = i;
      i = (x_82 + 1);
    }
  }
  p = vec2<i32>(0, 0);
  canwalk = true;
  v = 0;
  loop {
    var x_104 : bool;
    var x_124 : bool;
    var x_144 : bool;
    var x_164 : bool;
    var x_105_phi : bool;
    var x_125_phi : bool;
    var x_145_phi : bool;
    var x_165_phi : bool;
    let x_88 : i32 = v;
    v = (x_88 + 1);
    directions = 0;
    let x_91 : i32 = p.x;
    let x_92 : bool = (x_91 > 0);
    x_105_phi = x_92;
    if (x_92) {
      let x_96 : i32 = p.x;
      let x_99 : i32 = p.y;
      let x_103 : i32 = map[((x_96 - 2) + (x_99 * 16))];
      x_104 = (x_103 == 0);
      x_105_phi = x_104;
    }
    let x_105 : bool = x_105_phi;
    if (x_105) {
      let x_108 : i32 = directions;
      directions = (x_108 + 1);
    }
    let x_111 : i32 = p.y;
    let x_112 : bool = (x_111 > 0);
    x_125_phi = x_112;
    if (x_112) {
      let x_116 : i32 = p.x;
      let x_118 : i32 = p.y;
      let x_123 : i32 = map[(x_116 + ((x_118 - 2) * 16))];
      x_124 = (x_123 == 0);
      x_125_phi = x_124;
    }
    let x_125 : bool = x_125_phi;
    if (x_125) {
      let x_128 : i32 = directions;
      directions = (x_128 + 1);
    }
    let x_131 : i32 = p.x;
    let x_132 : bool = (x_131 < 14);
    x_145_phi = x_132;
    if (x_132) {
      let x_136 : i32 = p.x;
      let x_139 : i32 = p.y;
      let x_143 : i32 = map[((x_136 + 2) + (x_139 * 16))];
      x_144 = (x_143 == 0);
      x_145_phi = x_144;
    }
    let x_145 : bool = x_145_phi;
    if (x_145) {
      let x_148 : i32 = directions;
      directions = (x_148 + 1);
    }
    let x_151 : i32 = p.y;
    let x_152 : bool = (x_151 < 14);
    x_165_phi = x_152;
    if (x_152) {
      let x_156 : i32 = p.x;
      let x_158 : i32 = p.y;
      let x_163 : i32 = map[(x_156 + ((x_158 + 2) * 16))];
      x_164 = (x_163 == 0);
      x_165_phi = x_164;
    }
    let x_165 : bool = x_165_phi;
    if (x_165) {
      let x_168 : i32 = directions;
      directions = (x_168 + 1);
    }
    var x_229 : bool;
    var x_242 : bool;
    var x_281 : bool;
    var x_288 : i32;
    var x_289 : i32;
    var x_295 : i32;
    var x_296 : i32;
    var x_303 : array<i32, 256u>;
    var x_304 : array<i32, 256u>;
    var x_315 : i32;
    var x_316 : i32;
    var x_359 : bool;
    var x_372 : bool;
    var x_411 : bool;
    var x_424 : bool;
    var x_230_phi : bool;
    var x_243_phi : bool;
    var x_282_phi : bool;
    var x_290_phi : i32;
    var x_297_phi : i32;
    var x_305_phi : array<i32, 256u>;
    var x_317_phi : i32;
    var x_360_phi : bool;
    var x_373_phi : bool;
    var x_412_phi : bool;
    var x_425_phi : bool;
    let x_170 : i32 = directions;
    if ((x_170 == 0)) {
      canwalk = false;
      i = 0;
      loop {
        let x_179 : i32 = i;
        if ((x_179 < 8)) {
        } else {
          break;
        }
        j = 0;
        loop {
          let x_186 : i32 = j;
          if ((x_186 < 8)) {
          } else {
            break;
          }
          let x_189 : i32 = j;
          let x_191 : i32 = i;
          let x_196 : i32 = map[((x_189 * 2) + ((x_191 * 2) * 16))];
          if ((x_196 == 0)) {
            let x_200 : i32 = j;
            p.x = (x_200 * 2);
            let x_203 : i32 = i;
            p.y = (x_203 * 2);
            canwalk = true;
          }

          continuing {
            let x_206 : i32 = j;
            j = (x_206 + 1);
          }
        }

        continuing {
          let x_208 : i32 = i;
          i = (x_208 + 1);
        }
      }
      let x_211 : i32 = p.x;
      let x_213 : i32 = p.y;
      map[(x_211 + (x_213 * 16))] = 1;
    } else {
      let x_217 : i32 = v;
      let x_218 : i32 = directions;
      d = (x_217 % x_218);
      let x_220 : i32 = directions;
      let x_221 : i32 = v;
      v = (x_221 + x_220);
      let x_223 : i32 = d;
      let x_224 : bool = (x_223 >= 0);
      x_230_phi = x_224;
      if (x_224) {
        let x_228 : i32 = p.x;
        x_229 = (x_228 > 0);
        x_230_phi = x_229;
      }
      let x_230 : bool = x_230_phi;
      x_243_phi = x_230;
      if (x_230) {
        let x_234 : i32 = p.x;
        let x_237 : i32 = p.y;
        let x_241 : i32 = map[((x_234 - 2) + (x_237 * 16))];
        x_242 = (x_241 == 0);
        x_243_phi = x_242;
      }
      let x_243 : bool = x_243_phi;
      if (x_243) {
        let x_246 : i32 = d;
        d = (x_246 - 1);
        let x_249 : i32 = p.x;
        let x_251 : i32 = p.y;
        map[(x_249 + (x_251 * 16))] = 1;
        let x_256 : i32 = p.x;
        let x_259 : i32 = p.y;
        map[((x_256 - 1) + (x_259 * 16))] = 1;
        let x_264 : i32 = p.x;
        let x_267 : i32 = p.y;
        map[((x_264 - 2) + (x_267 * 16))] = 1;
        let x_272 : i32 = p.x;
        p.x = (x_272 - 2);
      }
      let x_275 : i32 = d;
      let x_276 : bool = (x_275 >= 0);
      x_282_phi = x_276;
      if (x_276) {
        let x_280 : i32 = p.y;
        x_281 = (x_280 > 0);
        x_282_phi = x_281;
      }
      let x_282 : bool = x_282_phi;
      if (x_282) {
        x_288 = p.x;
        x_290_phi = x_288;
      } else {
        x_289 = 0;
        x_290_phi = x_289;
      }
      let x_290 : i32 = x_290_phi;
      if (x_282) {
        x_295 = p.y;
        x_297_phi = x_295;
      } else {
        x_296 = 0;
        x_297_phi = x_296;
      }
      let x_297 : i32 = x_297_phi;
      let x_299 : i32 = ((x_297 - 2) * 16);
      if (x_282) {
        x_303 = map;
        x_305_phi = x_303;
      } else {
        x_304 = array<i32, 256u>(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        x_305_phi = x_304;
      }
      let x_305 : array<i32, 256u> = x_305_phi;
      if (x_282) {
        map = array<i32, 256u>(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      }
      if (x_282) {
        map = x_305;
      }
      if (x_282) {
        x_315 = map[(x_290 + x_299)];
        x_317_phi = x_315;
      } else {
        x_316 = 0;
        x_317_phi = x_316;
      }
      let x_317 : i32 = x_317_phi;
      let x_318 : bool = (x_317 == 0);
      if (select(x_282, x_318, x_282)) {
        let x_323 : i32 = d;
        d = (x_323 - 1);
        let x_326 : i32 = p.x;
        let x_328 : i32 = p.y;
        map[(x_326 + (x_328 * 16))] = 1;
        let x_333 : i32 = p.x;
        let x_335 : i32 = p.y;
        map[(x_333 + ((x_335 - 1) * 16))] = 1;
        let x_341 : i32 = p.x;
        let x_343 : i32 = p.y;
        let x_345 : array<i32, 256u> = map;
        map = array<i32, 256u>(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        map = x_345;
        map[(x_341 + ((x_343 - 2) * 16))] = 1;
        let x_350 : i32 = p.y;
        p.y = (x_350 - 2);
      }
      let x_353 : i32 = d;
      let x_354 : bool = (x_353 >= 0);
      x_360_phi = x_354;
      if (x_354) {
        let x_358 : i32 = p.x;
        x_359 = (x_358 < 14);
        x_360_phi = x_359;
      }
      let x_360 : bool = x_360_phi;
      x_373_phi = x_360;
      if (x_360) {
        let x_364 : i32 = p.x;
        let x_367 : i32 = p.y;
        let x_371 : i32 = map[((x_364 + 2) + (x_367 * 16))];
        x_372 = (x_371 == 0);
        x_373_phi = x_372;
      }
      let x_373 : bool = x_373_phi;
      if (x_373) {
        let x_376 : i32 = d;
        d = (x_376 - 1);
        let x_379 : i32 = p.x;
        let x_381 : i32 = p.y;
        map[(x_379 + (x_381 * 16))] = 1;
        let x_386 : i32 = p.x;
        let x_389 : i32 = p.y;
        map[((x_386 + 1) + (x_389 * 16))] = 1;
        let x_394 : i32 = p.x;
        let x_397 : i32 = p.y;
        map[((x_394 + 2) + (x_397 * 16))] = 1;
        let x_402 : i32 = p.x;
        p.x = (x_402 + 2);
      }
      let x_405 : i32 = d;
      let x_406 : bool = (x_405 >= 0);
      x_412_phi = x_406;
      if (x_406) {
        let x_410 : i32 = p.y;
        x_411 = (x_410 < 14);
        x_412_phi = x_411;
      }
      let x_412 : bool = x_412_phi;
      x_425_phi = x_412;
      if (x_412) {
        let x_416 : i32 = p.x;
        let x_418 : i32 = p.y;
        let x_423 : i32 = map[(x_416 + ((x_418 + 2) * 16))];
        x_424 = (x_423 == 0);
        x_425_phi = x_424;
      }
      let x_425 : bool = x_425_phi;
      if (x_425) {
        let x_428 : i32 = d;
        d = (x_428 - 1);
        let x_431 : i32 = p.x;
        let x_433 : i32 = p.y;
        map[(x_431 + (x_433 * 16))] = 1;
        let x_438 : i32 = p.x;
        let x_440 : i32 = p.y;
        map[(x_438 + ((x_440 + 1) * 16))] = 1;
        let x_446 : i32 = p.x;
        let x_448 : i32 = p.y;
        map[(x_446 + ((x_448 + 2) * 16))] = 1;
        let x_454 : i32 = p.y;
        p.y = (x_454 + 2);
      }
    }
    let x_458 : i32 = ipos.y;
    let x_461 : i32 = ipos.x;
    let x_464 : i32 = map[((x_458 * 16) + x_461)];
    if ((x_464 == 1)) {
      x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
      return;
    }

    continuing {
      let x_468 : bool = canwalk;
      break if !(x_468);
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
