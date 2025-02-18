struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var temp : array<i32, 10u>;
  var data : array<i32, 10u>;
  var x_190 : f32;
  var x_262 : f32;
  var x_63_phi : i32;
  var x_103_phi : i32;
  var x_112_phi : i32;
  var x_263_phi : f32;
  let x_60 : f32 = x_8.injectionSwitch.x;
  let x_61 : i32 = i32(x_60);
  x_63_phi = x_61;
  loop {
    var x_100 : i32;
    var x_98 : i32;
    var x_96 : i32;
    var x_94 : i32;
    var x_92 : i32;
    var x_90 : i32;
    var x_88 : i32;
    var x_86 : i32;
    var x_84 : i32;
    var x_82 : i32;
    var x_64_phi : i32;
    let x_63 : i32 = x_63_phi;
    let x_68 : array<i32, 10u> = data;
    data = array<i32, 10u>(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    data = x_68;
    let x_69 : i32 = (x_63 + 1);
    x_64_phi = x_69;
    switch(x_63) {
      case 9: {
        data[x_63] = -5;
        x_100 = (x_63 + 1);
        x_64_phi = x_100;
      }
      case 8: {
        data[x_63] = -4;
        x_98 = (x_63 + 1);
        x_64_phi = x_98;
      }
      case 7: {
        data[x_63] = -3;
        x_96 = (x_63 + 1);
        x_64_phi = x_96;
      }
      case 6: {
        data[x_63] = -2;
        x_94 = (x_63 + 1);
        x_64_phi = x_94;
      }
      case 5: {
        data[x_63] = -1;
        x_92 = (x_63 + 1);
        x_64_phi = x_92;
      }
      case 4: {
        data[x_63] = 0;
        x_90 = (x_63 + 1);
        x_64_phi = x_90;
      }
      case 3: {
        data[x_63] = 1;
        x_88 = (x_63 + 1);
        x_64_phi = x_88;
      }
      case 2: {
        data[x_63] = 2;
        x_86 = (x_63 + 1);
        x_64_phi = x_86;
      }
      case 1: {
        data[x_63] = 3;
        x_84 = (x_63 + 1);
        x_64_phi = x_84;
      }
      case 0: {
        data[x_63] = 4;
        x_82 = (x_63 + 1);
        x_64_phi = x_82;
      }
      default: {
      }
    }
    let x_64 : i32 = x_64_phi;

    continuing {
      x_63_phi = x_64;
      break if !(x_64 < 10);
    }
  }
  x_103_phi = 0;
  loop {
    var x_104 : i32;
    let x_103 : i32 = x_103_phi;
    if ((x_103 < 10)) {
    } else {
      break;
    }

    continuing {
      let x_109 : i32 = data[x_103];
      temp[x_103] = x_109;
      x_104 = (x_103 + 1);
      x_103_phi = x_104;
    }
  }
  x_112_phi = 1;
  loop {
    var x_113 : i32;
    var x_119_phi : i32;
    let x_112 : i32 = x_112_phi;
    if ((x_112 <= 9)) {
    } else {
      break;
    }
    x_119_phi = 0;
    loop {
      var x_131 : i32;
      var x_136 : i32;
      var x_131_phi : i32;
      var x_134_phi : i32;
      var x_136_phi : i32;
      var x_158_phi : i32;
      var x_161_phi : i32;
      var x_171_phi : i32;
      let x_119 : i32 = x_119_phi;
      if ((x_119 < 9)) {
      } else {
        break;
      }
      let x_125 : i32 = (x_119 + x_112);
      let x_126 : i32 = (x_125 - 1);
      let x_120 : i32 = (x_119 + (2 * x_112));
      let x_129 : i32 = min((x_120 - 1), 9);
      x_131_phi = x_119;
      x_134_phi = x_125;
      x_136_phi = x_119;
      loop {
        var x_151 : i32;
        var x_154 : i32;
        var x_135_phi : i32;
        var x_137_phi : i32;
        x_131 = x_131_phi;
        let x_134 : i32 = x_134_phi;
        x_136 = x_136_phi;
        if (((x_136 <= x_126) & (x_134 <= x_129))) {
        } else {
          break;
        }
        let x_143_save = x_136;
        let x_144 : i32 = data[x_143_save];
        let x_145_save = x_134;
        let x_146 : i32 = data[x_145_save];
        let x_132 : i32 = bitcast<i32>((x_131 + bitcast<i32>(1)));
        if ((x_144 < x_146)) {
          x_151 = bitcast<i32>((x_136 + bitcast<i32>(1)));
          let x_152 : i32 = data[x_143_save];
          temp[x_131] = x_152;
          x_135_phi = x_134;
          x_137_phi = x_151;
        } else {
          x_154 = (x_134 + 1);
          let x_155 : i32 = data[x_145_save];
          temp[x_131] = x_155;
          x_135_phi = x_154;
          x_137_phi = x_136;
        }
        let x_135 : i32 = x_135_phi;
        let x_137 : i32 = x_137_phi;

        continuing {
          x_131_phi = x_132;
          x_134_phi = x_135;
          x_136_phi = x_137;
        }
      }
      x_158_phi = x_131;
      x_161_phi = x_136;
      loop {
        var x_159 : i32;
        var x_162 : i32;
        let x_158 : i32 = x_158_phi;
        let x_161 : i32 = x_161_phi;
        if (((x_161 < 10) & (x_161 <= x_126))) {
        } else {
          break;
        }

        continuing {
          x_159 = (x_158 + 1);
          x_162 = (x_161 + 1);
          let x_168 : i32 = data[x_161];
          temp[x_158] = x_168;
          x_158_phi = x_159;
          x_161_phi = x_162;
        }
      }
      x_171_phi = x_119;
      loop {
        var x_172 : i32;
        let x_171 : i32 = x_171_phi;
        if ((x_171 <= x_129)) {
        } else {
          break;
        }

        continuing {
          let x_177 : i32 = temp[x_171];
          data[x_171] = x_177;
          x_172 = (x_171 + 1);
          x_171_phi = x_172;
        }
      }

      continuing {
        x_119_phi = x_120;
      }
    }

    continuing {
      x_113 = (2 * x_112);
      x_112_phi = x_113;
    }
  }
  var x_181 : i32;
  var x_199 : f32;
  var x_261 : f32;
  var x_262_phi : f32;
  let x_180 : f32 = gl_FragCoord.y;
  x_181 = i32(x_180);
  if ((x_181 < 30)) {
    let x_187 : i32 = data[0];
    x_190 = (0.5 + (f32(x_187) * 0.100000001));
    x_263_phi = x_190;
  } else {
    var x_208 : f32;
    var x_260 : f32;
    var x_261_phi : f32;
    if ((x_181 < 60)) {
      let x_196 : i32 = data[1];
      x_199 = (0.5 + (f32(x_196) * 0.100000001));
      x_262_phi = x_199;
    } else {
      var x_217 : f32;
      var x_259 : f32;
      var x_260_phi : f32;
      if ((x_181 < 90)) {
        let x_205 : i32 = data[2];
        x_208 = (0.5 + (f32(x_205) * 0.100000001));
        x_261_phi = x_208;
      } else {
        if ((x_181 < 120)) {
          let x_214 : i32 = data[3];
          x_217 = (0.5 + (f32(x_214) * 0.100000001));
          x_260_phi = x_217;
        } else {
          var x_230 : f32;
          var x_258 : f32;
          var x_259_phi : f32;
          if ((x_181 < 150)) {
            discard;
          } else {
            var x_239 : f32;
            var x_257 : f32;
            var x_258_phi : f32;
            if ((x_181 < 180)) {
              let x_227 : i32 = data[5];
              x_230 = (0.5 + (f32(x_227) * 0.100000001));
              x_259_phi = x_230;
            } else {
              var x_248 : f32;
              var x_256 : f32;
              var x_257_phi : f32;
              if ((x_181 < 210)) {
                let x_236 : i32 = data[6];
                x_239 = (0.5 + (f32(x_236) * 0.100000001));
                x_258_phi = x_239;
              } else {
                if ((x_181 < 240)) {
                  let x_245 : i32 = data[7];
                  x_248 = (0.5 + (f32(x_245) * 0.100000001));
                  x_257_phi = x_248;
                } else {
                  if ((x_181 < 270)) {
                  } else {
                    discard;
                  }
                  let x_253 : i32 = data[8];
                  x_256 = (0.5 + (f32(x_253) * 0.100000001));
                  x_257_phi = x_256;
                }
                x_257 = x_257_phi;
                x_258_phi = x_257;
              }
              x_258 = x_258_phi;
              x_259_phi = x_258;
            }
            x_259 = x_259_phi;
          }
          x_260_phi = x_259;
        }
        x_260 = x_260_phi;
        x_261_phi = x_260;
      }
      x_261 = x_261_phi;
      x_262_phi = x_261;
    }
    x_262 = x_262_phi;
    x_263_phi = x_262;
  }
  let x_263 : f32 = x_263_phi;
  x_GLF_color = vec4<f32>(x_263, x_263, x_263, 1.0);
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
