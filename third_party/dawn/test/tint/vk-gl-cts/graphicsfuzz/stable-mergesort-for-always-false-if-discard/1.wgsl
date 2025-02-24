struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var temp : array<i32, 10u>;
  var data : array<i32, 10u>;
  var x_180 : f32;
  var x_279 : f32;
  var x_65_phi : i32;
  var x_93_phi : i32;
  var x_102_phi : i32;
  var x_280_phi : f32;
  let x_62 : f32 = x_8.injectionSwitch.x;
  let x_63 : i32 = i32(x_62);
  x_65_phi = x_63;
  loop {
    let x_65 : i32 = x_65_phi;
    switch(x_65) {
      case 9: {
        data[x_65] = -5;
      }
      case 8: {
        data[x_65] = -4;
      }
      case 7: {
        data[x_65] = -3;
      }
      case 6: {
        data[x_65] = -2;
      }
      case 5: {
        data[x_65] = -1;
      }
      case 4: {
        data[x_65] = 0;
      }
      case 3: {
        data[x_65] = 1;
      }
      case 2: {
        data[x_65] = 2;
      }
      case 1: {
        data[x_65] = 3;
      }
      case 0: {
        data[x_65] = 4;
      }
      default: {
      }
    }
    let x_66 : i32 = (x_65 + 1);

    continuing {
      x_65_phi = x_66;
      break if !(x_66 < 10);
    }
  }
  x_93_phi = 0;
  loop {
    var x_94 : i32;
    let x_93 : i32 = x_93_phi;
    if ((x_93 < 10)) {
    } else {
      break;
    }

    continuing {
      let x_99 : i32 = data[x_93];
      temp[x_93] = x_99;
      x_94 = (x_93 + 1);
      x_93_phi = x_94;
    }
  }
  x_102_phi = 1;
  loop {
    var x_103 : i32;
    var x_109_phi : i32;
    let x_102 : i32 = x_102_phi;
    if ((x_102 <= 9)) {
    } else {
      break;
    }
    x_109_phi = 0;
    loop {
      var x_121 : i32;
      var x_126 : i32;
      var x_121_phi : i32;
      var x_124_phi : i32;
      var x_126_phi : i32;
      var x_148_phi : i32;
      var x_151_phi : i32;
      var x_161_phi : i32;
      let x_109 : i32 = x_109_phi;
      if ((x_109 < 9)) {
      } else {
        break;
      }
      let x_115 : i32 = (x_109 + x_102);
      let x_116 : i32 = (x_115 - 1);
      let x_110 : i32 = (x_109 + (2 * x_102));
      let x_119 : i32 = min((x_110 - 1), 9);
      x_121_phi = x_109;
      x_124_phi = x_115;
      x_126_phi = x_109;
      loop {
        var x_141 : i32;
        var x_144 : i32;
        var x_125_phi : i32;
        var x_127_phi : i32;
        x_121 = x_121_phi;
        let x_124 : i32 = x_124_phi;
        x_126 = x_126_phi;
        if (((x_126 <= x_116) & (x_124 <= x_119))) {
        } else {
          break;
        }
        let x_133_save = x_126;
        let x_134 : i32 = data[x_133_save];
        let x_135_save = x_124;
        let x_136 : i32 = data[x_135_save];
        let x_122 : i32 = bitcast<i32>((x_121 + bitcast<i32>(1)));
        if ((x_134 < x_136)) {
          x_141 = bitcast<i32>((x_126 + bitcast<i32>(1)));
          let x_142 : i32 = data[x_133_save];
          temp[x_121] = x_142;
          x_125_phi = x_124;
          x_127_phi = x_141;
        } else {
          x_144 = (x_124 + 1);
          let x_145 : i32 = data[x_135_save];
          temp[x_121] = x_145;
          x_125_phi = x_144;
          x_127_phi = x_126;
        }
        let x_125 : i32 = x_125_phi;
        let x_127 : i32 = x_127_phi;

        continuing {
          x_121_phi = x_122;
          x_124_phi = x_125;
          x_126_phi = x_127;
        }
      }
      x_148_phi = x_121;
      x_151_phi = x_126;
      loop {
        var x_149 : i32;
        var x_152 : i32;
        let x_148 : i32 = x_148_phi;
        let x_151 : i32 = x_151_phi;
        if (((x_151 < 10) & (x_151 <= x_116))) {
        } else {
          break;
        }

        continuing {
          x_149 = (x_148 + 1);
          x_152 = (x_151 + 1);
          let x_158 : i32 = data[x_151];
          temp[x_148] = x_158;
          x_148_phi = x_149;
          x_151_phi = x_152;
        }
      }
      x_161_phi = x_109;
      loop {
        var x_162 : i32;
        let x_161 : i32 = x_161_phi;
        if ((x_161 <= x_119)) {
        } else {
          break;
        }

        continuing {
          let x_167 : i32 = temp[x_161];
          data[x_161] = x_167;
          x_162 = (x_161 + 1);
          x_161_phi = x_162;
        }
      }

      continuing {
        x_109_phi = x_110;
      }
    }

    continuing {
      x_103 = (2 * x_102);
      x_102_phi = x_103;
    }
  }
  var x_171 : i32;
  var x_189 : f32;
  var x_278 : f32;
  var x_279_phi : f32;
  let x_170 : f32 = gl_FragCoord.y;
  x_171 = i32(x_170);
  if ((x_171 < 30)) {
    let x_177 : i32 = data[0];
    x_180 = (0.5 + (f32(x_177) * 0.100000001));
    x_280_phi = x_180;
  } else {
    var x_198 : f32;
    var x_277 : f32;
    var x_278_phi : f32;
    if ((x_171 < 60)) {
      let x_186 : i32 = data[1];
      x_189 = (0.5 + (f32(x_186) * 0.100000001));
      x_279_phi = x_189;
    } else {
      var x_207 : f32;
      var x_249 : f32;
      var x_277_phi : f32;
      if ((x_171 < 90)) {
        let x_195 : i32 = data[2];
        x_198 = (0.5 + (f32(x_195) * 0.100000001));
        x_278_phi = x_198;
      } else {
        if ((x_171 < 120)) {
          let x_204 : i32 = data[3];
          x_207 = (0.5 + (f32(x_204) * 0.100000001));
          x_277_phi = x_207;
        } else {
          var x_220 : f32;
          var x_248 : f32;
          var x_249_phi : f32;
          var x_256_phi : vec2<f32>;
          var x_259_phi : i32;
          if ((x_171 < 150)) {
            discard;
          } else {
            var x_229 : f32;
            var x_247 : f32;
            var x_248_phi : f32;
            if ((x_171 < 180)) {
              let x_217 : i32 = data[5];
              x_220 = (0.5 + (f32(x_217) * 0.100000001));
              x_249_phi = x_220;
            } else {
              var x_238 : f32;
              var x_246 : f32;
              var x_247_phi : f32;
              if ((x_171 < 210)) {
                let x_226 : i32 = data[6];
                x_229 = (0.5 + (f32(x_226) * 0.100000001));
                x_248_phi = x_229;
              } else {
                if ((x_171 < 240)) {
                  let x_235 : i32 = data[7];
                  x_238 = (0.5 + (f32(x_235) * 0.100000001));
                  x_247_phi = x_238;
                } else {
                  if ((x_171 < 270)) {
                  } else {
                    discard;
                  }
                  let x_243 : i32 = data[8];
                  x_246 = (0.5 + (f32(x_243) * 0.100000001));
                  x_247_phi = x_246;
                }
                x_247 = x_247_phi;
                x_248_phi = x_247;
              }
              x_248 = x_248_phi;
              x_249_phi = x_248;
            }
            x_249 = x_249_phi;
            let x_251 : f32 = x_8.injectionSwitch.y;
            let x_252 : bool = (x_62 > x_251);
            if (x_252) {
              x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
            }
            x_256_phi = vec2<f32>(1.0, 1.0);
            x_259_phi = 0;
            loop {
              var x_272 : vec2<f32>;
              var x_260 : i32;
              var x_273_phi : vec2<f32>;
              let x_256 : vec2<f32> = x_256_phi;
              let x_259 : i32 = x_259_phi;
              if ((x_259 <= 32)) {
              } else {
                break;
              }
              x_273_phi = x_256;
              if ((x_256.x < 0.0)) {
                if (x_252) {
                  discard;
                }
                x_272 = x_256;
                x_272.y = (x_256.y + 1.0);
                x_273_phi = x_272;
              }
              let x_273 : vec2<f32> = x_273_phi;
              var x_257_1 : vec2<f32> = x_273;
              x_257_1.x = (x_273.x + x_273.y);
              let x_257 : vec2<f32> = x_257_1;

              continuing {
                x_260 = (x_259 + 1);
                x_256_phi = x_257;
                x_259_phi = x_260;
              }
            }
          }
          x_277_phi = x_249;
        }
        x_277 = x_277_phi;
        x_278_phi = x_277;
      }
      x_278 = x_278_phi;
      x_279_phi = x_278;
    }
    x_279 = x_279_phi;
    x_280_phi = x_279;
  }
  let x_280 : f32 = x_280_phi;
  x_GLF_color = vec4<f32>(x_280, x_280, x_280, 1.0);
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
