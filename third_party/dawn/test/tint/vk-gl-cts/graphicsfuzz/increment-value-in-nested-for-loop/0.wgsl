struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  loop {
    var x_45 : bool;
    var x_48 : i32;
    var x_49 : i32;
    var x_46 : bool;
    var x_115 : i32;
    var x_116 : i32;
    var x_45_phi : bool;
    var x_48_phi : i32;
    var x_50_phi : i32;
    var x_52_phi : i32;
    var x_111_phi : i32;
    var x_112_phi : bool;
    var x_115_phi : i32;
    var x_118_phi : i32;
    var x_120_phi : i32;
    var x_161_phi : i32;
    let x_40 : f32 = x_6.injectionSwitch.x;
    let x_41 : bool = (x_40 < -1.0);
    x_45_phi = false;
    x_48_phi = 0;
    x_50_phi = 0;
    x_52_phi = 0;
    loop {
      var x_62 : i32;
      var x_65 : i32;
      var x_66 : i32;
      var x_63 : i32;
      var x_53 : i32;
      var x_62_phi : i32;
      var x_65_phi : i32;
      var x_67_phi : i32;
      var x_51_phi : i32;
      var x_49_phi : i32;
      var x_46_phi : bool;
      x_45 = x_45_phi;
      x_48 = x_48_phi;
      let x_50 : i32 = x_50_phi;
      let x_52 : i32 = x_52_phi;
      let x_55 : f32 = gl_FragCoord.y;
      x_111_phi = x_48;
      x_112_phi = x_45;
      if ((x_52 < select(100, 10, (x_55 > -1.0)))) {
      } else {
        break;
      }
      x_62_phi = x_48;
      x_65_phi = x_50;
      x_67_phi = 0;
      loop {
        var x_97 : i32;
        var x_68 : i32;
        var x_66_phi : i32;
        x_62 = x_62_phi;
        x_65 = x_65_phi;
        let x_67 : i32 = x_67_phi;
        x_51_phi = x_65;
        x_49_phi = x_62;
        x_46_phi = x_45;
        if ((x_67 < 2)) {
        } else {
          break;
        }
        loop {
          var x_78 : bool;
          var x_86_phi : i32;
          var x_97_phi : i32;
          var x_98_phi : bool;
          let x_77 : f32 = gl_FragCoord.x;
          x_78 = (x_77 < -1.0);
          if (!((x_40 < 0.0))) {
            if (x_78) {
              x_66_phi = 0;
              break;
            }
            x_86_phi = 1;
            loop {
              var x_87 : i32;
              let x_86 : i32 = x_86_phi;
              x_97_phi = x_65;
              x_98_phi = false;
              if ((x_86 < 3)) {
              } else {
                break;
              }
              if (x_78) {
                continue;
              }
              if ((x_86 > 0)) {
                x_97_phi = 1;
                x_98_phi = true;
                break;
              }

              continuing {
                x_87 = (x_86 + 1);
                x_86_phi = x_87;
              }
            }
            x_97 = x_97_phi;
            let x_98 : bool = x_98_phi;
            x_66_phi = x_97;
            if (x_98) {
              break;
            }
          }
          x_66_phi = 0;
          break;
        }
        x_66 = x_66_phi;
        x_63 = bitcast<i32>((x_62 + x_66));
        if (x_41) {
          loop {
            if (x_41) {
            } else {
              break;
            }

            continuing {
              let x_105 : f32 = f32(x_52);
              x_GLF_color = vec4<f32>(x_105, x_105, x_105, x_105);
            }
          }
          x_51_phi = x_66;
          x_49_phi = x_63;
          x_46_phi = true;
          break;
        }

        continuing {
          x_68 = (x_67 + 1);
          x_62_phi = x_63;
          x_65_phi = x_66;
          x_67_phi = x_68;
        }
      }
      let x_51 : i32 = x_51_phi;
      x_49 = x_49_phi;
      x_46 = x_46_phi;
      x_111_phi = x_49;
      x_112_phi = x_46;
      if (x_46) {
        break;
      }
      if (!(x_41)) {
        x_111_phi = x_49;
        x_112_phi = x_46;
        break;
      }

      continuing {
        x_53 = (x_52 + 1);
        x_45_phi = x_46;
        x_48_phi = x_49;
        x_50_phi = x_51;
        x_52_phi = x_53;
      }
    }
    let x_111 : i32 = x_111_phi;
    let x_112 : bool = x_112_phi;
    if (x_112) {
      break;
    }
    x_115_phi = x_111;
    x_118_phi = 0;
    x_120_phi = 0;
    loop {
      var x_154 : i32;
      var x_121 : i32;
      var x_119_phi : i32;
      x_115 = x_115_phi;
      let x_118 : i32 = x_118_phi;
      let x_120 : i32 = x_120_phi;
      let x_123 : f32 = x_6.injectionSwitch.y;
      x_161_phi = x_115;
      if ((x_120 < i32((x_123 + 1.0)))) {
      } else {
        break;
      }
      loop {
        var x_135 : bool;
        var x_143_phi : i32;
        var x_154_phi : i32;
        var x_155_phi : bool;
        let x_134 : f32 = gl_FragCoord.x;
        x_135 = (x_134 < -1.0);
        if (!((x_40 < 0.0))) {
          if (x_135) {
            x_119_phi = 0;
            break;
          }
          x_143_phi = 1;
          loop {
            var x_144 : i32;
            let x_143 : i32 = x_143_phi;
            x_154_phi = x_118;
            x_155_phi = false;
            if ((x_143 < 3)) {
            } else {
              break;
            }
            if (x_135) {
              continue;
            }
            if ((x_143 > 0)) {
              x_154_phi = 1;
              x_155_phi = true;
              break;
            }

            continuing {
              x_144 = (x_143 + 1);
              x_143_phi = x_144;
            }
          }
          x_154 = x_154_phi;
          let x_155 : bool = x_155_phi;
          x_119_phi = x_154;
          if (x_155) {
            break;
          }
        }
        x_119_phi = 0;
        break;
      }
      var x_119 : i32;
      x_119 = x_119_phi;
      x_116 = bitcast<i32>((x_115 + x_119));
      if (select(x_41, false, !(x_41))) {
        x_161_phi = x_116;
        break;
      }

      continuing {
        x_121 = (x_120 + 1);
        x_115_phi = x_116;
        x_118_phi = x_119;
        x_120_phi = x_121;
      }
    }
    let x_161 : i32 = x_161_phi;
    if ((x_161 == 4)) {
      x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    } else {
      x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
    }
    break;
  }
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
