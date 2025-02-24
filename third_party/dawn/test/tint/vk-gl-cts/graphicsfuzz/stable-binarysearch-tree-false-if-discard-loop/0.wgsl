struct BST {
  data : i32,
  leftIndex : i32,
  rightIndex : i32,
}

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var tree : array<BST, 10u>;
  var x_356 : i32;
  var x_58_phi : i32;
  var x_86_phi : bool;
  var x_353_phi : i32;
  var x_356_phi : i32;
  var x_358_phi : i32;
  tree[0] = BST(9, -1, -1);
  switch(0u) {
    default: {
      x_58_phi = 0;
      loop {
        var x_84 : i32;
        var x_76 : i32;
        var x_59 : i32;
        var x_59_phi : i32;
        let x_58 : i32 = x_58_phi;
        x_86_phi = false;
        if ((x_58 <= 1)) {
        } else {
          break;
        }
        let x_65 : i32 = tree[x_58].data;
        if ((5 <= x_65)) {
          let x_78_save = x_58;
          let x_79 : i32 = tree[x_78_save].leftIndex;
          if ((x_79 == -1)) {
            tree[x_78_save].leftIndex = 1;
            tree[1] = BST(5, -1, -1);
            x_86_phi = true;
            break;
          } else {
            x_84 = tree[x_78_save].leftIndex;
            x_59_phi = x_84;
            continue;
          }
        } else {
          let x_70_save = x_58;
          let x_71 : i32 = tree[x_70_save].rightIndex;
          if ((x_71 == -1)) {
            tree[x_70_save].rightIndex = 1;
            tree[1] = BST(5, -1, -1);
            x_86_phi = true;
            break;
          } else {
            x_76 = tree[x_70_save].rightIndex;
            x_59_phi = x_76;
            continue;
          }
        }

        continuing {
          x_59 = x_59_phi;
          x_58_phi = x_59;
        }
      }
      let x_86 : bool = x_86_phi;
      if (x_86) {
        break;
      }
    }
  }
  var x_91_phi : i32;
  var x_119_phi : bool;
  switch(0u) {
    default: {
      x_91_phi = 0;
      loop {
        var x_117 : i32;
        var x_109 : i32;
        var x_92 : i32;
        var x_92_phi : i32;
        let x_91 : i32 = x_91_phi;
        x_119_phi = false;
        if ((x_91 <= 2)) {
        } else {
          break;
        }
        let x_98 : i32 = tree[x_91].data;
        if ((12 <= x_98)) {
          let x_111_save = x_91;
          let x_112 : i32 = tree[x_111_save].leftIndex;
          if ((x_112 == -1)) {
            tree[x_111_save].leftIndex = 2;
            tree[2] = BST(12, -1, -1);
            x_119_phi = true;
            break;
          } else {
            x_117 = tree[x_111_save].leftIndex;
            x_92_phi = x_117;
            continue;
          }
        } else {
          let x_103_save = x_91;
          let x_104 : i32 = tree[x_103_save].rightIndex;
          if ((x_104 == -1)) {
            tree[x_103_save].rightIndex = 2;
            tree[2] = BST(12, -1, -1);
            x_119_phi = true;
            break;
          } else {
            x_109 = tree[x_103_save].rightIndex;
            x_92_phi = x_109;
            continue;
          }
        }

        continuing {
          x_92 = x_92_phi;
          x_91_phi = x_92;
        }
      }
      let x_119 : bool = x_119_phi;
      if (x_119) {
        break;
      }
    }
  }
  var x_124_phi : i32;
  var x_152_phi : bool;
  switch(0u) {
    default: {
      x_124_phi = 0;
      loop {
        var x_150 : i32;
        var x_142 : i32;
        var x_125 : i32;
        var x_125_phi : i32;
        let x_124 : i32 = x_124_phi;
        x_152_phi = false;
        if ((x_124 <= 3)) {
        } else {
          break;
        }
        let x_131 : i32 = tree[x_124].data;
        if ((15 <= x_131)) {
          let x_144_save = x_124;
          let x_145 : i32 = tree[x_144_save].leftIndex;
          if ((x_145 == -1)) {
            tree[x_144_save].leftIndex = 3;
            tree[3] = BST(15, -1, -1);
            x_152_phi = true;
            break;
          } else {
            x_150 = tree[x_144_save].leftIndex;
            x_125_phi = x_150;
            continue;
          }
        } else {
          let x_136_save = x_124;
          let x_137 : i32 = tree[x_136_save].rightIndex;
          if ((x_137 == -1)) {
            tree[x_136_save].rightIndex = 3;
            tree[3] = BST(15, -1, -1);
            x_152_phi = true;
            break;
          } else {
            x_142 = tree[x_136_save].rightIndex;
            x_125_phi = x_142;
            continue;
          }
        }

        continuing {
          x_125 = x_125_phi;
          x_124_phi = x_125;
        }
      }
      let x_152 : bool = x_152_phi;
      if (x_152) {
        break;
      }
    }
  }
  var x_157_phi : i32;
  var x_185_phi : bool;
  switch(0u) {
    default: {
      x_157_phi = 0;
      loop {
        var x_183 : i32;
        var x_175 : i32;
        var x_158 : i32;
        var x_158_phi : i32;
        let x_157 : i32 = x_157_phi;
        x_185_phi = false;
        if ((x_157 <= 4)) {
        } else {
          break;
        }
        let x_164 : i32 = tree[x_157].data;
        if ((7 <= x_164)) {
          let x_177_save = x_157;
          let x_178 : i32 = tree[x_177_save].leftIndex;
          if ((x_178 == -1)) {
            tree[x_177_save].leftIndex = 4;
            tree[4] = BST(7, -1, -1);
            x_185_phi = true;
            break;
          } else {
            x_183 = tree[x_177_save].leftIndex;
            x_158_phi = x_183;
            continue;
          }
        } else {
          let x_169_save = x_157;
          let x_170 : i32 = tree[x_169_save].rightIndex;
          if ((x_170 == -1)) {
            tree[x_169_save].rightIndex = 4;
            tree[4] = BST(7, -1, -1);
            x_185_phi = true;
            break;
          } else {
            x_175 = tree[x_169_save].rightIndex;
            x_158_phi = x_175;
            continue;
          }
        }

        continuing {
          x_158 = x_158_phi;
          x_157_phi = x_158;
        }
      }
      let x_185 : bool = x_185_phi;
      if (x_185) {
        break;
      }
    }
  }
  var x_190_phi : i32;
  var x_218_phi : bool;
  switch(0u) {
    default: {
      x_190_phi = 0;
      loop {
        var x_216 : i32;
        var x_208 : i32;
        var x_191 : i32;
        var x_191_phi : i32;
        let x_190 : i32 = x_190_phi;
        x_218_phi = false;
        if ((x_190 <= 5)) {
        } else {
          break;
        }
        let x_197 : i32 = tree[x_190].data;
        if ((8 <= x_197)) {
          let x_210_save = x_190;
          let x_211 : i32 = tree[x_210_save].leftIndex;
          if ((x_211 == -1)) {
            tree[x_210_save].leftIndex = 5;
            tree[5] = BST(8, -1, -1);
            x_218_phi = true;
            break;
          } else {
            x_216 = tree[x_210_save].leftIndex;
            x_191_phi = x_216;
            continue;
          }
        } else {
          let x_202_save = x_190;
          let x_203 : i32 = tree[x_202_save].rightIndex;
          if ((x_203 == -1)) {
            tree[x_202_save].rightIndex = 5;
            tree[5] = BST(8, -1, -1);
            x_218_phi = true;
            break;
          } else {
            x_208 = tree[x_202_save].rightIndex;
            x_191_phi = x_208;
            continue;
          }
        }

        continuing {
          x_191 = x_191_phi;
          x_190_phi = x_191;
        }
      }
      let x_218 : bool = x_218_phi;
      if (x_218) {
        break;
      }
    }
  }
  var x_223_phi : i32;
  var x_251_phi : bool;
  switch(0u) {
    default: {
      x_223_phi = 0;
      loop {
        var x_249 : i32;
        var x_241 : i32;
        var x_224 : i32;
        var x_224_phi : i32;
        let x_223 : i32 = x_223_phi;
        x_251_phi = false;
        if ((x_223 <= 6)) {
        } else {
          break;
        }
        let x_230 : i32 = tree[x_223].data;
        if ((2 <= x_230)) {
          let x_243_save = x_223;
          let x_244 : i32 = tree[x_243_save].leftIndex;
          if ((x_244 == -1)) {
            tree[x_243_save].leftIndex = 6;
            tree[6] = BST(2, -1, -1);
            x_251_phi = true;
            break;
          } else {
            x_249 = tree[x_243_save].leftIndex;
            x_224_phi = x_249;
            continue;
          }
        } else {
          let x_235_save = x_223;
          let x_236 : i32 = tree[x_235_save].rightIndex;
          if ((x_236 == -1)) {
            tree[x_235_save].rightIndex = 6;
            tree[6] = BST(2, -1, -1);
            x_251_phi = true;
            break;
          } else {
            x_241 = tree[x_235_save].rightIndex;
            x_224_phi = x_241;
            continue;
          }
        }

        continuing {
          x_224 = x_224_phi;
          x_223_phi = x_224;
        }
      }
      let x_251 : bool = x_251_phi;
      if (x_251) {
        break;
      }
    }
  }
  var x_256_phi : i32;
  var x_284_phi : bool;
  switch(0u) {
    default: {
      x_256_phi = 0;
      loop {
        var x_282 : i32;
        var x_274 : i32;
        var x_257 : i32;
        var x_257_phi : i32;
        let x_256 : i32 = x_256_phi;
        x_284_phi = false;
        if ((x_256 <= 7)) {
        } else {
          break;
        }
        let x_263 : i32 = tree[x_256].data;
        if ((6 <= x_263)) {
          let x_276_save = x_256;
          let x_277 : i32 = tree[x_276_save].leftIndex;
          if ((x_277 == -1)) {
            tree[x_276_save].leftIndex = 7;
            tree[7] = BST(6, -1, -1);
            x_284_phi = true;
            break;
          } else {
            x_282 = tree[x_276_save].leftIndex;
            x_257_phi = x_282;
            continue;
          }
        } else {
          let x_268_save = x_256;
          let x_269 : i32 = tree[x_268_save].rightIndex;
          if ((x_269 == -1)) {
            tree[x_268_save].rightIndex = 7;
            tree[7] = BST(6, -1, -1);
            x_284_phi = true;
            break;
          } else {
            x_274 = tree[x_268_save].rightIndex;
            x_257_phi = x_274;
            continue;
          }
        }

        continuing {
          x_257 = x_257_phi;
          x_256_phi = x_257;
        }
      }
      let x_284 : bool = x_284_phi;
      if (x_284) {
        break;
      }
    }
  }
  var x_289_phi : i32;
  var x_317_phi : bool;
  switch(0u) {
    default: {
      x_289_phi = 0;
      loop {
        var x_315 : i32;
        var x_307 : i32;
        var x_290 : i32;
        var x_290_phi : i32;
        let x_289 : i32 = x_289_phi;
        x_317_phi = false;
        if ((x_289 <= 8)) {
        } else {
          break;
        }
        let x_296 : i32 = tree[x_289].data;
        if ((17 <= x_296)) {
          let x_309_save = x_289;
          let x_310 : i32 = tree[x_309_save].leftIndex;
          if ((x_310 == -1)) {
            tree[x_309_save].leftIndex = 8;
            tree[8] = BST(17, -1, -1);
            x_317_phi = true;
            break;
          } else {
            x_315 = tree[x_309_save].leftIndex;
            x_290_phi = x_315;
            continue;
          }
        } else {
          let x_301_save = x_289;
          let x_302 : i32 = tree[x_301_save].rightIndex;
          if ((x_302 == -1)) {
            tree[x_301_save].rightIndex = 8;
            tree[8] = BST(17, -1, -1);
            x_317_phi = true;
            break;
          } else {
            x_307 = tree[x_301_save].rightIndex;
            x_290_phi = x_307;
            continue;
          }
        }

        continuing {
          x_290 = x_290_phi;
          x_289_phi = x_290;
        }
      }
      let x_317 : bool = x_317_phi;
      if (x_317) {
        break;
      }
    }
  }
  var x_322_phi : i32;
  var x_350_phi : bool;
  switch(0u) {
    default: {
      x_322_phi = 0;
      loop {
        var x_348 : i32;
        var x_340 : i32;
        var x_323 : i32;
        var x_323_phi : i32;
        let x_322 : i32 = x_322_phi;
        x_350_phi = false;
        if ((x_322 <= 9)) {
        } else {
          break;
        }
        let x_329 : i32 = tree[x_322].data;
        if ((13 <= x_329)) {
          let x_342_save = x_322;
          let x_343 : i32 = tree[x_342_save].leftIndex;
          if ((x_343 == -1)) {
            tree[x_342_save].leftIndex = 9;
            tree[9] = BST(13, -1, -1);
            x_350_phi = true;
            break;
          } else {
            x_348 = tree[x_342_save].leftIndex;
            x_323_phi = x_348;
            continue;
          }
        } else {
          let x_334_save = x_322;
          let x_335 : i32 = tree[x_334_save].rightIndex;
          if ((x_335 == -1)) {
            tree[x_334_save].rightIndex = 9;
            tree[9] = BST(13, -1, -1);
            x_350_phi = true;
            break;
          } else {
            x_340 = tree[x_334_save].rightIndex;
            x_323_phi = x_340;
            continue;
          }
        }

        continuing {
          x_323 = x_323_phi;
          x_322_phi = x_323;
        }
      }
      let x_350 : bool = x_350_phi;
      if (x_350) {
        break;
      }
    }
  }
  x_353_phi = 0;
  x_356_phi = 0;
  x_358_phi = 0;
  loop {
    var x_381 : i32;
    var x_391 : i32;
    var x_396 : i32;
    var x_359 : i32;
    var x_354_phi : i32;
    var x_357_phi : i32;
    let x_353 : i32 = x_353_phi;
    x_356 = x_356_phi;
    let x_358 : i32 = x_358_phi;
    if ((x_358 < 20)) {
    } else {
      break;
    }
    var x_366_phi : i32;
    var x_381_phi : i32;
    var x_382_phi : bool;
    switch(0u) {
      default: {
        x_366_phi = 0;
        loop {
          let x_366 : i32 = x_366_phi;
          x_381_phi = x_353;
          x_382_phi = false;
          if ((x_366 != -1)) {
          } else {
            break;
          }
          let x_373 : BST = tree[x_366];
          let x_374 : i32 = x_373.data;
          let x_375 : i32 = x_373.leftIndex;
          let x_376 : i32 = x_373.rightIndex;
          if ((x_374 == x_358)) {
            x_381_phi = x_358;
            x_382_phi = true;
            break;
          }

          continuing {
            x_366_phi = select(x_375, x_376, (x_358 > x_374));
          }
        }
        x_381 = x_381_phi;
        let x_382 : bool = x_382_phi;
        x_354_phi = x_381;
        if (x_382) {
          break;
        }
        x_354_phi = -1;
      }
    }
    var x_354 : i32;
    var x_390 : i32;
    var x_395 : i32;
    var x_391_phi : i32;
    var x_396_phi : i32;
    x_354 = x_354_phi;
    switch(x_358) {
      case 2, 5, 6, 7, 8, 9, 12, 13, 15, 17: {
        x_391_phi = x_356;
        if ((x_354 == bitcast<i32>(x_358))) {
          x_390 = bitcast<i32>((x_356 + bitcast<i32>(1)));
          x_391_phi = x_390;
        }
        x_391 = x_391_phi;
        x_357_phi = x_391;
      }
      default: {
        x_396_phi = x_356;
        if ((x_354 == bitcast<i32>(-1))) {
          x_395 = bitcast<i32>((x_356 + bitcast<i32>(1)));
          x_396_phi = x_395;
        }
        x_396 = x_396_phi;
        x_357_phi = x_396;
      }
    }
    let x_357 : i32 = x_357_phi;

    continuing {
      x_359 = (x_358 + 1);
      x_353_phi = x_354;
      x_356_phi = x_357;
      x_358_phi = x_359;
    }
  }
  if ((x_356 == bitcast<i32>(20))) {
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
