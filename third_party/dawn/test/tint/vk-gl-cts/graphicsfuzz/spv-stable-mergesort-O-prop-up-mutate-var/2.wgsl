struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var temp : array<i32, 10u>;
  var data : array<i32, 10u>;
  var x_189 : f32;
  var x_261 : f32;
  var x_63_phi : i32;
  var x_102_phi : i32;
  var x_111_phi : i32;
  var x_262_phi : f32;
  let x_60 : f32 = x_8.injectionSwitch.x;
  let x_61 : i32 = i32(x_60);
  x_63_phi = x_61;
  loop {
    var x_99 : i32;
    var x_97 : i32;
    var x_95 : i32;
    var x_93 : i32;
    var x_91 : i32;
    var x_89 : i32;
    var x_87 : i32;
    var x_85 : i32;
    var x_83 : i32;
    var x_81 : i32;
    var x_64_phi : i32;
    let x_63 : i32 = x_63_phi;
    let x_68 : i32 = (x_63 + 1);
    x_64_phi = x_68;
    switch(x_63) {
      case 9: {
        data[x_63] = -5;
        x_99 = (x_63 + 1);
        x_64_phi = x_99;
      }
      case 8: {
        data[x_63] = -4;
        x_97 = (x_63 + 1);
        x_64_phi = x_97;
      }
      case 7: {
        data[x_63] = -3;
        x_95 = (x_63 + 1);
        x_64_phi = x_95;
      }
      case 6: {
        data[x_63] = -2;
        x_93 = (x_63 + 1);
        x_64_phi = x_93;
      }
      case 5: {
        data[x_63] = -1;
        x_91 = (x_63 + 1);
        x_64_phi = x_91;
      }
      case 4: {
        data[x_63] = 0;
        x_89 = (x_63 + 1);
        x_64_phi = x_89;
      }
      case 3: {
        data[x_63] = 1;
        x_87 = (x_63 + 1);
        x_64_phi = x_87;
      }
      case 2: {
        data[x_63] = 2;
        x_85 = (x_63 + 1);
        x_64_phi = x_85;
      }
      case 1: {
        data[x_63] = 3;
        x_83 = (x_63 + 1);
        x_64_phi = x_83;
      }
      case 0: {
        data[x_63] = 4;
        x_81 = (x_63 + 1);
        x_64_phi = x_81;
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
  x_102_phi = 0;
  loop {
    var x_103 : i32;
    let x_102 : i32 = x_102_phi;
    if ((x_102 < 10)) {
    } else {
      break;
    }

    continuing {
      let x_108 : i32 = data[x_102];
      temp[x_102] = x_108;
      x_103 = (x_102 + 1);
      x_102_phi = x_103;
    }
  }
  x_111_phi = 1;
  loop {
    var x_112 : i32;
    var x_118_phi : i32;
    let x_111 : i32 = x_111_phi;
    if ((x_111 <= 9)) {
    } else {
      break;
    }
    x_118_phi = 0;
    loop {
      var x_130 : i32;
      var x_135 : i32;
      var x_130_phi : i32;
      var x_133_phi : i32;
      var x_135_phi : i32;
      var x_157_phi : i32;
      var x_160_phi : i32;
      var x_170_phi : i32;
      let x_118 : i32 = x_118_phi;
      if ((x_118 < 9)) {
      } else {
        break;
      }
      let x_124 : i32 = (x_118 + x_111);
      let x_125 : i32 = (x_124 - 1);
      let x_119 : i32 = (x_118 + (2 * x_111));
      let x_128 : i32 = min((x_119 - 1), 9);
      x_130_phi = x_118;
      x_133_phi = x_124;
      x_135_phi = x_118;
      loop {
        var x_150 : i32;
        var x_153 : i32;
        var x_134_phi : i32;
        var x_136_phi : i32;
        x_130 = x_130_phi;
        let x_133 : i32 = x_133_phi;
        x_135 = x_135_phi;
        if (((x_135 <= x_125) & (x_133 <= x_128))) {
        } else {
          break;
        }
        let x_142_save = x_135;
        let x_143 : i32 = data[x_142_save];
        let x_144_save = x_133;
        let x_145 : i32 = data[x_144_save];
        let x_131 : i32 = bitcast<i32>((x_130 + bitcast<i32>(1)));
        if ((x_143 < x_145)) {
          x_150 = bitcast<i32>((x_135 + bitcast<i32>(1)));
          let x_151 : i32 = data[x_142_save];
          temp[x_130] = x_151;
          x_134_phi = x_133;
          x_136_phi = x_150;
        } else {
          x_153 = (x_133 + 1);
          let x_154 : i32 = data[x_144_save];
          temp[x_130] = x_154;
          x_134_phi = x_153;
          x_136_phi = x_135;
        }
        let x_134 : i32 = x_134_phi;
        let x_136 : i32 = x_136_phi;

        continuing {
          x_130_phi = x_131;
          x_133_phi = x_134;
          x_135_phi = x_136;
        }
      }
      x_157_phi = x_130;
      x_160_phi = x_135;
      loop {
        var x_158 : i32;
        var x_161 : i32;
        let x_157 : i32 = x_157_phi;
        let x_160 : i32 = x_160_phi;
        if (((x_160 < 10) & (x_160 <= x_125))) {
        } else {
          break;
        }

        continuing {
          x_158 = (x_157 + 1);
          x_161 = (x_160 + 1);
          let x_167 : i32 = data[x_160];
          temp[x_157] = x_167;
          x_157_phi = x_158;
          x_160_phi = x_161;
        }
      }
      x_170_phi = x_118;
      loop {
        var x_171 : i32;
        let x_170 : i32 = x_170_phi;
        if ((x_170 <= x_128)) {
        } else {
          break;
        }

        continuing {
          let x_176 : i32 = temp[x_170];
          data[x_170] = x_176;
          x_171 = (x_170 + 1);
          x_170_phi = x_171;
        }
      }

      continuing {
        x_118_phi = x_119;
      }
    }

    continuing {
      x_112 = (2 * x_111);
      x_111_phi = x_112;
    }
  }
  var x_180 : i32;
  var x_198 : f32;
  var x_260 : f32;
  var x_261_phi : f32;
  let x_179 : f32 = gl_FragCoord.y;
  x_180 = i32(x_179);
  if ((x_180 < 30)) {
    let x_186 : i32 = data[0];
    x_189 = (0.5 + (f32(x_186) * 0.100000001));
    x_262_phi = x_189;
  } else {
    var x_207 : f32;
    var x_259 : f32;
    var x_260_phi : f32;
    if ((x_180 < 60)) {
      let x_195 : i32 = data[1];
      x_198 = (0.5 + (f32(x_195) * 0.100000001));
      x_261_phi = x_198;
    } else {
      var x_216 : f32;
      var x_258 : f32;
      var x_259_phi : f32;
      if ((x_180 < 90)) {
        let x_204 : i32 = data[2];
        x_207 = (0.5 + (f32(x_204) * 0.100000001));
        x_260_phi = x_207;
      } else {
        if ((x_180 < 120)) {
          let x_213 : i32 = data[3];
          x_216 = (0.5 + (f32(x_213) * 0.100000001));
          x_259_phi = x_216;
        } else {
          var x_229 : f32;
          var x_257 : f32;
          var x_258_phi : f32;
          if ((x_180 < 150)) {
            discard;
          } else {
            var x_238 : f32;
            var x_256 : f32;
            var x_257_phi : f32;
            if ((x_180 < 180)) {
              let x_226 : i32 = data[5];
              x_229 = (0.5 + (f32(x_226) * 0.100000001));
              x_258_phi = x_229;
            } else {
              var x_247 : f32;
              var x_255 : f32;
              var x_256_phi : f32;
              if ((x_180 < 210)) {
                let x_235 : i32 = data[6];
                x_238 = (0.5 + (f32(x_235) * 0.100000001));
                x_257_phi = x_238;
              } else {
                if ((x_180 < 240)) {
                  let x_244 : i32 = data[7];
                  x_247 = (0.5 + (f32(x_244) * 0.100000001));
                  x_256_phi = x_247;
                } else {
                  if ((x_180 < 270)) {
                  } else {
                    discard;
                  }
                  let x_252 : i32 = data[8];
                  x_255 = (0.5 + (f32(x_252) * 0.100000001));
                  x_256_phi = x_255;
                }
                x_256 = x_256_phi;
                x_257_phi = x_256;
              }
              x_257 = x_257_phi;
              x_258_phi = x_257;
            }
            x_258 = x_258_phi;
          }
          x_259_phi = x_258;
        }
        x_259 = x_259_phi;
        x_260_phi = x_259;
      }
      x_260 = x_260_phi;
      x_261_phi = x_260;
    }
    x_261 = x_261_phi;
    x_262_phi = x_261;
  }
  let x_262 : f32 = x_262_phi;
  x_GLF_color = vec4<f32>(x_262, x_262, x_262, 1.0);
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
