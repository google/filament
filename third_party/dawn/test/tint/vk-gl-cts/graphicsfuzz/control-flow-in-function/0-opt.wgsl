struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_25 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn drawShape_vf2_(pos : ptr<function, vec2<f32>>) -> vec3<f32> {
  var c2 : bool;
  var c3 : bool;
  var c4 : bool;
  var c5 : bool;
  var c6 : bool;
  var GLF_live4i : i32;
  var GLF_live4_looplimiter5 : i32;
  var GLF_live7m42 : mat4x2<f32>;
  var GLF_live7m33 : mat3x3<f32>;
  var GLF_live7cols : i32;
  var GLF_live7_looplimiter3 : i32;
  var GLF_live7rows : i32;
  var GLF_live7_looplimiter2 : i32;
  var GLF_live7_looplimiter1 : i32;
  var GLF_live7c : i32;
  var GLF_live7r : i32;
  var GLF_live7_looplimiter0 : i32;
  var GLF_live7sum_index : i32;
  var GLF_live7_looplimiter7 : i32;
  var GLF_live7cols_1 : i32;
  var GLF_live7rows_1 : i32;
  var GLF_live7sums : array<f32, 9u>;
  var GLF_live7c_1 : i32;
  var GLF_live7r_1 : i32;
  var x_180 : i32;
  var indexable : mat3x3<f32>;
  let x_182 : f32 = (*(pos)).x;
  c2 = (x_182 > 1.0);
  let x_184 : bool = c2;
  if (x_184) {
    return vec3<f32>(1.0, 1.0, 1.0);
  }
  let x_188 : f32 = (*(pos)).y;
  c3 = (x_188 < 1.0);
  let x_190 : bool = c3;
  if (x_190) {
    return vec3<f32>(1.0, 1.0, 1.0);
  }
  let x_194 : f32 = (*(pos)).y;
  c4 = (x_194 > 1.0);
  let x_196 : bool = c4;
  if (x_196) {
    return vec3<f32>(1.0, 1.0, 1.0);
  }
  let x_200 : f32 = (*(pos)).x;
  c5 = (x_200 < 1.0);
  let x_202 : bool = c5;
  if (x_202) {
    return vec3<f32>(1.0, 1.0, 1.0);
  }
  let x_206 : f32 = (*(pos)).x;
  c6 = ((x_206 + 1.0) > 1.0);
  let x_209 : bool = c6;
  if (x_209) {
    return vec3<f32>(1.0, 1.0, 1.0);
  }
  GLF_live4i = 0;
  loop {
    let x_39 : i32 = GLF_live4i;
    if ((x_39 < 4)) {
    } else {
      break;
    }
    let x_40 : i32 = GLF_live4_looplimiter5;
    if ((x_40 >= 7)) {
      break;
    }
    let x_41 : i32 = GLF_live4_looplimiter5;
    GLF_live4_looplimiter5 = (x_41 + 1);
    GLF_live7m42 = mat4x2<f32>(vec2<f32>(1.0, 0.0), vec2<f32>(0.0, 1.0), vec2<f32>(0.0, 0.0), vec2<f32>(1.0, 0.0));
    GLF_live7m33 = mat3x3<f32>(vec3<f32>(1.0, 0.0, 0.0), vec3<f32>(0.0, 1.0, 0.0), vec3<f32>(0.0, 0.0, 1.0));
    GLF_live7cols = 2;
    loop {
      let x_43 : i32 = GLF_live7cols;
      if ((x_43 < 4)) {
      } else {
        break;
      }
      let x_44 : i32 = GLF_live7_looplimiter3;
      if ((x_44 >= 7)) {
        break;
      }
      let x_45 : i32 = GLF_live7_looplimiter3;
      GLF_live7_looplimiter3 = (x_45 + 1);
      GLF_live7rows = 2;
      loop {
        let x_47 : i32 = GLF_live7rows;
        if ((x_47 < 4)) {
        } else {
          break;
        }
        let x_48 : i32 = GLF_live7_looplimiter2;
        if ((x_48 >= 7)) {
          break;
        }
        let x_49 : i32 = GLF_live7_looplimiter2;
        GLF_live7_looplimiter2 = (x_49 + 1);
        GLF_live7_looplimiter1 = 0;
        GLF_live7c = 0;
        loop {
          let x_51 : i32 = GLF_live7c;
          if ((x_51 < 3)) {
          } else {
            break;
          }
          let x_52 : i32 = GLF_live7_looplimiter1;
          if ((x_52 >= 7)) {
            break;
          }
          let x_53 : i32 = GLF_live7_looplimiter1;
          GLF_live7_looplimiter1 = (x_53 + 1);
          GLF_live7r = 0;
          loop {
            let x_55 : i32 = GLF_live7r;
            if ((x_55 < 2)) {
            } else {
              break;
            }
            let x_56 : i32 = GLF_live7_looplimiter0;
            if ((x_56 >= 7)) {
              break;
            }
            let x_57 : i32 = GLF_live7_looplimiter0;
            GLF_live7_looplimiter0 = (x_57 + 1);
            let x_59 : i32 = GLF_live7c;
            let x_60 : i32 = GLF_live7c;
            let x_61 : i32 = GLF_live7c;
            let x_62 : i32 = GLF_live7r;
            let x_63 : i32 = GLF_live7r;
            let x_64 : i32 = GLF_live7r;
            GLF_live7m33[select(0, x_61, ((x_59 >= 0) & (x_60 < 3)))][select(0, x_64, ((x_62 >= 0) & (x_63 < 3)))] = 1.0;
            let x_267 : f32 = x_25.injectionSwitch.y;
            if ((0.0 > x_267)) {
            } else {
              let x_65 : i32 = GLF_live7c;
              let x_66 : i32 = GLF_live7c;
              let x_67 : i32 = GLF_live7c;
              let x_68 : i32 = GLF_live7r;
              let x_69 : i32 = GLF_live7r;
              let x_70 : i32 = GLF_live7r;
              GLF_live7m42[select(0, x_67, ((x_65 >= 0) & (x_66 < 4)))][select(0, x_70, ((x_68 >= 0) & (x_69 < 2)))] = 1.0;
            }

            continuing {
              let x_71 : i32 = GLF_live7r;
              GLF_live7r = (x_71 + 1);
            }
          }

          continuing {
            let x_73 : i32 = GLF_live7c;
            GLF_live7c = (x_73 + 1);
          }
        }

        continuing {
          let x_75 : i32 = GLF_live7rows;
          GLF_live7rows = (x_75 + 1);
        }
      }

      continuing {
        let x_77 : i32 = GLF_live7cols;
        GLF_live7cols = (x_77 + 1);
      }
    }
    GLF_live7sum_index = 0;
    GLF_live7_looplimiter7 = 0;
    GLF_live7cols_1 = 2;
    loop {
      let x_79 : i32 = GLF_live7cols_1;
      if ((x_79 < 4)) {
      } else {
        break;
      }
      let x_80 : i32 = GLF_live7_looplimiter7;
      if ((x_80 >= 7)) {
        break;
      }
      let x_81 : i32 = GLF_live7_looplimiter7;
      GLF_live7_looplimiter7 = (x_81 + 1);
      GLF_live7rows_1 = 2;
      let x_83 : i32 = GLF_live7sum_index;
      let x_84 : i32 = GLF_live7sum_index;
      let x_85 : i32 = GLF_live7sum_index;
      GLF_live7sums[select(0, x_85, ((x_83 >= 0) & (x_84 < 9)))] = 0.0;
      GLF_live7c_1 = 0;
      loop {
        let x_86 : i32 = GLF_live7c_1;
        if ((x_86 < 1)) {
        } else {
          break;
        }
        GLF_live7r_1 = 0;
        loop {
          let x_87 : i32 = GLF_live7r_1;
          let x_88 : i32 = GLF_live7rows_1;
          if ((x_87 < x_88)) {
          } else {
            break;
          }
          let x_89 : i32 = GLF_live7sum_index;
          let x_90 : i32 = GLF_live7sum_index;
          let x_91 : i32 = GLF_live7sum_index;
          let x_310 : i32 = select(0, x_91, ((x_89 >= 0) & (x_90 < 9)));
          let x_311 : mat3x3<f32> = GLF_live7m33;
          let x_312 : mat3x3<f32> = transpose(x_311);
          let x_92 : i32 = GLF_live7c_1;
          if ((x_92 < 3)) {
            x_180 = 1;
          } else {
            let x_318 : f32 = x_25.injectionSwitch.x;
            x_180 = i32(x_318);
          }
          let x_320 : i32 = x_180;
          let x_93 : i32 = GLF_live7r_1;
          indexable = x_312;
          let x_324 : f32 = indexable[x_320][select(0, 1, (x_93 < 3))];
          let x_326 : f32 = GLF_live7sums[x_310];
          GLF_live7sums[x_310] = (x_326 + x_324);
          let x_94 : i32 = GLF_live7sum_index;
          let x_95 : i32 = GLF_live7sum_index;
          let x_96 : i32 = GLF_live7sum_index;
          let x_332 : i32 = select(0, x_96, ((x_94 >= 0) & (x_95 < 9)));
          let x_97 : i32 = GLF_live7r_1;
          let x_334 : f32 = GLF_live7m42[1][x_97];
          let x_336 : f32 = GLF_live7sums[x_332];
          GLF_live7sums[x_332] = (x_336 + x_334);

          continuing {
            let x_98 : i32 = GLF_live7r_1;
            GLF_live7r_1 = (x_98 + 1);
          }
        }

        continuing {
          let x_100 : i32 = GLF_live7c_1;
          GLF_live7c_1 = (x_100 + 1);
        }
      }
      let x_102 : i32 = GLF_live7sum_index;
      GLF_live7sum_index = (x_102 + 1);

      continuing {
        let x_104 : i32 = GLF_live7cols_1;
        GLF_live7cols_1 = (x_104 + 1);
      }
    }

    continuing {
      let x_106 : i32 = GLF_live4i;
      GLF_live4i = (x_106 + 1);
    }
  }
  return vec3<f32>(1.0, 1.0, 1.0);
}

fn main_1() {
  var position : vec2<f32>;
  var param : vec2<f32>;
  var param_1 : vec2<f32>;
  var i : i32;
  var param_2 : vec2<f32>;
  let x_161 : f32 = x_25.injectionSwitch.x;
  if ((x_161 >= 2.0)) {
    let x_165 : vec4<f32> = gl_FragCoord;
    position = vec2<f32>(x_165.x, x_165.y);
    let x_167 : vec2<f32> = position;
    param = x_167;
    let x_168 : vec3<f32> = drawShape_vf2_(&(param));
    let x_169 : vec2<f32> = position;
    param_1 = x_169;
    let x_170 : vec3<f32> = drawShape_vf2_(&(param_1));
    i = 25;
    loop {
      let x_108 : i32 = i;
      if ((x_108 > 0)) {
      } else {
        break;
      }
      let x_177 : vec2<f32> = position;
      param_2 = x_177;
      let x_178 : vec3<f32> = drawShape_vf2_(&(param_2));

      continuing {
        let x_109 : i32 = i;
        i = (x_109 - 1);
      }
    }
  }
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
