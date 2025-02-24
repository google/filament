struct BST {
  data : i32,
  leftIndex : i32,
  rightIndex : i32,
}

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var tree : array<BST, 10u>;
  var x_360 : i32;
  var x_62_phi : i32;
  var x_90_phi : bool;
  var x_357_phi : i32;
  var x_360_phi : i32;
  var x_362_phi : i32;
  tree[0] = BST(9, -1, -1);
  switch(0u) {
    default: {
      x_62_phi = 0;
      loop {
        var x_88 : i32;
        var x_80 : i32;
        var x_63 : i32;
        var x_63_phi : i32;
        let x_62 : i32 = x_62_phi;
        x_90_phi = false;
        if ((x_62 <= 1)) {
        } else {
          break;
        }
        let x_69 : i32 = tree[x_62].data;
        if ((5 <= x_69)) {
          let x_82_save = x_62;
          let x_83 : i32 = tree[x_82_save].leftIndex;
          if ((x_83 == -1)) {
            tree[x_82_save].leftIndex = 1;
            tree[1] = BST(5, -1, -1);
            x_90_phi = true;
            break;
          } else {
            x_88 = tree[x_82_save].leftIndex;
            x_63_phi = x_88;
            continue;
          }
        } else {
          let x_74_save = x_62;
          let x_75 : i32 = tree[x_74_save].rightIndex;
          if ((x_75 == -1)) {
            tree[x_74_save].rightIndex = 1;
            tree[1] = BST(5, -1, -1);
            x_90_phi = true;
            break;
          } else {
            x_80 = tree[x_74_save].rightIndex;
            x_63_phi = x_80;
            continue;
          }
        }

        continuing {
          x_63 = x_63_phi;
          x_62_phi = x_63;
        }
      }
      let x_90 : bool = x_90_phi;
      if (x_90) {
        break;
      }
    }
  }
  var x_95_phi : i32;
  var x_123_phi : bool;
  switch(0u) {
    default: {
      x_95_phi = 0;
      loop {
        var x_121 : i32;
        var x_113 : i32;
        var x_96 : i32;
        var x_96_phi : i32;
        let x_95 : i32 = x_95_phi;
        x_123_phi = false;
        if ((x_95 <= 2)) {
        } else {
          break;
        }
        let x_102 : i32 = tree[x_95].data;
        if ((12 <= x_102)) {
          let x_115_save = x_95;
          let x_116 : i32 = tree[x_115_save].leftIndex;
          if ((x_116 == -1)) {
            tree[x_115_save].leftIndex = 2;
            tree[2] = BST(12, -1, -1);
            x_123_phi = true;
            break;
          } else {
            x_121 = tree[x_115_save].leftIndex;
            x_96_phi = x_121;
            continue;
          }
        } else {
          let x_107_save = x_95;
          let x_108 : i32 = tree[x_107_save].rightIndex;
          if ((x_108 == -1)) {
            tree[x_107_save].rightIndex = 2;
            tree[2] = BST(12, -1, -1);
            x_123_phi = true;
            break;
          } else {
            x_113 = tree[x_107_save].rightIndex;
            x_96_phi = x_113;
            continue;
          }
        }

        continuing {
          x_96 = x_96_phi;
          x_95_phi = x_96;
        }
      }
      let x_123 : bool = x_123_phi;
      if (x_123) {
        break;
      }
    }
  }
  var x_128_phi : i32;
  var x_156_phi : bool;
  switch(0u) {
    default: {
      x_128_phi = 0;
      loop {
        var x_154 : i32;
        var x_146 : i32;
        var x_129 : i32;
        var x_129_phi : i32;
        let x_128 : i32 = x_128_phi;
        x_156_phi = false;
        if ((x_128 <= 3)) {
        } else {
          break;
        }
        let x_135 : i32 = tree[x_128].data;
        if ((15 <= x_135)) {
          let x_148_save = x_128;
          let x_149 : i32 = tree[x_148_save].leftIndex;
          if ((x_149 == -1)) {
            tree[x_148_save].leftIndex = 3;
            tree[3] = BST(15, -1, -1);
            x_156_phi = true;
            break;
          } else {
            x_154 = tree[x_148_save].leftIndex;
            x_129_phi = x_154;
            continue;
          }
        } else {
          let x_140_save = x_128;
          let x_141 : i32 = tree[x_140_save].rightIndex;
          if ((x_141 == -1)) {
            tree[x_140_save].rightIndex = 3;
            tree[3] = BST(15, -1, -1);
            x_156_phi = true;
            break;
          } else {
            x_146 = tree[x_140_save].rightIndex;
            x_129_phi = x_146;
            continue;
          }
        }

        continuing {
          x_129 = x_129_phi;
          x_128_phi = x_129;
        }
      }
      let x_156 : bool = x_156_phi;
      if (x_156) {
        break;
      }
    }
  }
  var x_161_phi : i32;
  var x_189_phi : bool;
  switch(0u) {
    default: {
      x_161_phi = 0;
      loop {
        var x_187 : i32;
        var x_179 : i32;
        var x_162 : i32;
        var x_162_phi : i32;
        let x_161 : i32 = x_161_phi;
        x_189_phi = false;
        if ((x_161 <= 4)) {
        } else {
          break;
        }
        let x_168 : i32 = tree[x_161].data;
        if ((7 <= x_168)) {
          let x_181_save = x_161;
          let x_182 : i32 = tree[x_181_save].leftIndex;
          if ((x_182 == -1)) {
            tree[x_181_save].leftIndex = 4;
            tree[4] = BST(7, -1, -1);
            x_189_phi = true;
            break;
          } else {
            x_187 = tree[x_181_save].leftIndex;
            x_162_phi = x_187;
            continue;
          }
        } else {
          let x_173_save = x_161;
          let x_174 : i32 = tree[x_173_save].rightIndex;
          if ((x_174 == -1)) {
            tree[x_173_save].rightIndex = 4;
            tree[4] = BST(7, -1, -1);
            x_189_phi = true;
            break;
          } else {
            x_179 = tree[x_173_save].rightIndex;
            x_162_phi = x_179;
            continue;
          }
        }

        continuing {
          x_162 = x_162_phi;
          x_161_phi = x_162;
        }
      }
      let x_189 : bool = x_189_phi;
      if (x_189) {
        break;
      }
    }
  }
  var x_194_phi : i32;
  var x_222_phi : bool;
  switch(0u) {
    default: {
      x_194_phi = 0;
      loop {
        var x_220 : i32;
        var x_212 : i32;
        var x_195 : i32;
        var x_195_phi : i32;
        let x_194 : i32 = x_194_phi;
        x_222_phi = false;
        if ((x_194 <= 5)) {
        } else {
          break;
        }
        let x_201 : i32 = tree[x_194].data;
        if ((8 <= x_201)) {
          let x_214_save = x_194;
          let x_215 : i32 = tree[x_214_save].leftIndex;
          if ((x_215 == -1)) {
            tree[x_214_save].leftIndex = 5;
            tree[5] = BST(8, -1, -1);
            x_222_phi = true;
            break;
          } else {
            x_220 = tree[x_214_save].leftIndex;
            x_195_phi = x_220;
            continue;
          }
        } else {
          let x_206_save = x_194;
          let x_207 : i32 = tree[x_206_save].rightIndex;
          if ((x_207 == -1)) {
            tree[x_206_save].rightIndex = 5;
            tree[5] = BST(8, -1, -1);
            x_222_phi = true;
            break;
          } else {
            x_212 = tree[x_206_save].rightIndex;
            x_195_phi = x_212;
            continue;
          }
        }

        continuing {
          x_195 = x_195_phi;
          x_194_phi = x_195;
        }
      }
      let x_222 : bool = x_222_phi;
      if (x_222) {
        break;
      }
    }
  }
  var x_227_phi : i32;
  var x_255_phi : bool;
  switch(0u) {
    default: {
      x_227_phi = 0;
      loop {
        var x_253 : i32;
        var x_245 : i32;
        var x_228 : i32;
        var x_228_phi : i32;
        let x_227 : i32 = x_227_phi;
        x_255_phi = false;
        if ((x_227 <= 6)) {
        } else {
          break;
        }
        let x_234 : i32 = tree[x_227].data;
        if ((2 <= x_234)) {
          let x_247_save = x_227;
          let x_248 : i32 = tree[x_247_save].leftIndex;
          if ((x_248 == -1)) {
            tree[x_247_save].leftIndex = 6;
            tree[6] = BST(2, -1, -1);
            x_255_phi = true;
            break;
          } else {
            x_253 = tree[x_247_save].leftIndex;
            x_228_phi = x_253;
            continue;
          }
        } else {
          let x_239_save = x_227;
          let x_240 : i32 = tree[x_239_save].rightIndex;
          if ((x_240 == -1)) {
            tree[x_239_save].rightIndex = 6;
            tree[6] = BST(2, -1, -1);
            x_255_phi = true;
            break;
          } else {
            x_245 = tree[x_239_save].rightIndex;
            x_228_phi = x_245;
            continue;
          }
        }

        continuing {
          x_228 = x_228_phi;
          x_227_phi = x_228;
        }
      }
      let x_255 : bool = x_255_phi;
      if (x_255) {
        break;
      }
    }
  }
  var x_260_phi : i32;
  var x_288_phi : bool;
  switch(0u) {
    default: {
      x_260_phi = 0;
      loop {
        var x_286 : i32;
        var x_278 : i32;
        var x_261 : i32;
        var x_261_phi : i32;
        let x_260 : i32 = x_260_phi;
        x_288_phi = false;
        if ((x_260 <= 7)) {
        } else {
          break;
        }
        let x_267 : i32 = tree[x_260].data;
        if ((6 <= x_267)) {
          let x_280_save = x_260;
          let x_281 : i32 = tree[x_280_save].leftIndex;
          if ((x_281 == -1)) {
            tree[x_280_save].leftIndex = 7;
            tree[7] = BST(6, -1, -1);
            x_288_phi = true;
            break;
          } else {
            x_286 = tree[x_280_save].leftIndex;
            x_261_phi = x_286;
            continue;
          }
        } else {
          let x_272_save = x_260;
          let x_273 : i32 = tree[x_272_save].rightIndex;
          if ((x_273 == -1)) {
            tree[x_272_save].rightIndex = 7;
            tree[7] = BST(6, -1, -1);
            x_288_phi = true;
            break;
          } else {
            x_278 = tree[x_272_save].rightIndex;
            x_261_phi = x_278;
            continue;
          }
        }

        continuing {
          x_261 = x_261_phi;
          x_260_phi = x_261;
        }
      }
      let x_288 : bool = x_288_phi;
      if (x_288) {
        break;
      }
    }
  }
  var x_293_phi : i32;
  var x_321_phi : bool;
  switch(0u) {
    default: {
      x_293_phi = 0;
      loop {
        var x_319 : i32;
        var x_311 : i32;
        var x_294 : i32;
        var x_294_phi : i32;
        let x_293 : i32 = x_293_phi;
        x_321_phi = false;
        if ((x_293 <= 8)) {
        } else {
          break;
        }
        let x_300 : i32 = tree[x_293].data;
        if ((17 <= x_300)) {
          let x_313_save = x_293;
          let x_314 : i32 = tree[x_313_save].leftIndex;
          if ((x_314 == -1)) {
            tree[x_313_save].leftIndex = 8;
            tree[8] = BST(17, -1, -1);
            x_321_phi = true;
            break;
          } else {
            x_319 = tree[x_313_save].leftIndex;
            x_294_phi = x_319;
            continue;
          }
        } else {
          let x_305_save = x_293;
          let x_306 : i32 = tree[x_305_save].rightIndex;
          if ((x_306 == -1)) {
            tree[x_305_save].rightIndex = 8;
            tree[8] = BST(17, -1, -1);
            x_321_phi = true;
            break;
          } else {
            x_311 = tree[x_305_save].rightIndex;
            x_294_phi = x_311;
            continue;
          }
        }

        continuing {
          x_294 = x_294_phi;
          x_293_phi = x_294;
        }
      }
      let x_321 : bool = x_321_phi;
      if (x_321) {
        break;
      }
    }
  }
  var x_326_phi : i32;
  var x_354_phi : bool;
  switch(0u) {
    default: {
      x_326_phi = 0;
      loop {
        var x_352 : i32;
        var x_344 : i32;
        var x_327 : i32;
        var x_327_phi : i32;
        let x_326 : i32 = x_326_phi;
        x_354_phi = false;
        if ((x_326 <= 9)) {
        } else {
          break;
        }
        let x_333 : i32 = tree[x_326].data;
        if ((13 <= x_333)) {
          let x_346_save = x_326;
          let x_347 : i32 = tree[x_346_save].leftIndex;
          if ((x_347 == -1)) {
            tree[x_346_save].leftIndex = 9;
            tree[9] = BST(13, -1, -1);
            x_354_phi = true;
            break;
          } else {
            x_352 = tree[x_346_save].leftIndex;
            x_327_phi = x_352;
            continue;
          }
        } else {
          let x_338_save = x_326;
          let x_339 : i32 = tree[x_338_save].rightIndex;
          if ((x_339 == -1)) {
            tree[x_338_save].rightIndex = 9;
            tree[9] = BST(13, -1, -1);
            x_354_phi = true;
            break;
          } else {
            x_344 = tree[x_338_save].rightIndex;
            x_327_phi = x_344;
            continue;
          }
        }

        continuing {
          x_327 = x_327_phi;
          x_326_phi = x_327;
        }
      }
      let x_354 : bool = x_354_phi;
      if (x_354) {
        break;
      }
    }
  }
  x_357_phi = 0;
  x_360_phi = 0;
  x_362_phi = 0;
  loop {
    var x_392 : i32;
    var x_402 : i32;
    var x_407 : i32;
    var x_363 : i32;
    var x_358_phi : i32;
    var x_361_phi : i32;
    let x_357 : i32 = x_357_phi;
    x_360 = x_360_phi;
    let x_362 : i32 = x_362_phi;
    let x_365 : i32 = (6 - 15);
    if ((x_362 < 20)) {
    } else {
      break;
    }
    var x_374_phi : i32;
    var x_392_phi : i32;
    var x_393_phi : bool;
    switch(0u) {
      default: {
        x_374_phi = 0;
        loop {
          let x_374 : i32 = x_374_phi;
          x_392_phi = x_357;
          x_393_phi = false;
          if ((x_374 != -1)) {
          } else {
            break;
          }
          let x_381 : BST = tree[x_374];
          let x_382 : i32 = x_381.data;
          let x_383 : i32 = x_381.leftIndex;
          let x_385 : i32 = x_381.rightIndex;
          if ((x_382 == x_362)) {
            x_392_phi = x_362;
            x_393_phi = true;
            break;
          }
          let x_389 : f32 = x_GLF_color[select(3u, 3u, (3u <= 3u))];

          continuing {
            x_374_phi = select(x_383, x_385, !((x_362 <= x_382)));
          }
        }
        x_392 = x_392_phi;
        let x_393 : bool = x_393_phi;
        x_358_phi = x_392;
        if (x_393) {
          break;
        }
        x_358_phi = -1;
      }
    }
    var x_358 : i32;
    var x_401 : i32;
    var x_406 : i32;
    var x_402_phi : i32;
    var x_407_phi : i32;
    x_358 = x_358_phi;
    switch(x_362) {
      case 2, 5, 6, 7, 8, 9, 12, 13, 15, 17: {
        x_402_phi = x_360;
        if ((x_358 == bitcast<i32>(x_362))) {
          x_401 = bitcast<i32>((x_360 + bitcast<i32>(1)));
          x_402_phi = x_401;
        }
        x_402 = x_402_phi;
        x_361_phi = x_402;
      }
      default: {
        x_407_phi = x_360;
        if ((x_358 == bitcast<i32>(-1))) {
          x_406 = bitcast<i32>((x_360 + bitcast<i32>(1)));
          x_407_phi = x_406;
        }
        x_407 = x_407_phi;
        x_361_phi = x_407;
      }
    }
    let x_361 : i32 = x_361_phi;

    continuing {
      x_363 = (x_362 + 1);
      x_357_phi = x_358;
      x_360_phi = x_361;
      x_362_phi = x_363;
    }
  }
  if ((x_360 == bitcast<i32>(20))) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 1.0, 1.0);
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
