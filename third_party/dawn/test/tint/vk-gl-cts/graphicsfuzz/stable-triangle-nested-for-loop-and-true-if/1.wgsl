struct buf1 {
  injectionSwitch : vec2<f32>,
}

struct buf0 {
  resolution : vec2<f32>,
}

@group(0) @binding(1) var<uniform> x_11 : buf1;

var<private> x_GLF_color : vec4<f32>;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_19 : buf0;

fn pointInTriangle_vf2_vf2_vf2_vf2_(p : ptr<function, vec2<f32>>, a : ptr<function, vec2<f32>>, b : ptr<function, vec2<f32>>, c : ptr<function, vec2<f32>>) -> i32 {
  var x_78 : f32;
  var x_79 : f32;
  var x_80 : f32;
  var param : vec2<f32>;
  var param_1 : vec2<f32>;
  var param_2 : vec2<f32>;
  var param_3 : vec2<f32>;
  var param_4 : vec2<f32>;
  var param_5 : vec2<f32>;
  var x_147 : bool;
  var x_203 : bool;
  var x_148_phi : bool;
  var x_204_phi : bool;
  let x_82 : f32 = (*(p)).x;
  let x_84 : f32 = (*(a)).x;
  let x_87 : f32 = (*(p)).y;
  let x_89 : f32 = (*(a)).y;
  let x_93 : f32 = (*(b)).x;
  let x_94 : f32 = (*(a)).x;
  let x_97 : f32 = (*(b)).y;
  let x_98 : f32 = (*(a)).y;
  param = vec2<f32>((x_82 - x_84), (x_87 - x_89));
  param_1 = vec2<f32>((x_93 - x_94), (x_97 - x_98));
  let x_102 : f32 = param.x;
  let x_104 : f32 = param_1.y;
  let x_107 : f32 = param_1.x;
  let x_109 : f32 = param.y;
  let x_111 : f32 = ((x_102 * x_104) - (x_107 * x_109));
  x_80 = x_111;
  let x_112 : f32 = (*(p)).x;
  let x_113 : f32 = (*(b)).x;
  let x_115 : f32 = (*(p)).y;
  let x_116 : f32 = (*(b)).y;
  let x_120 : f32 = (*(c)).x;
  let x_121 : f32 = (*(b)).x;
  let x_124 : f32 = (*(c)).y;
  let x_125 : f32 = (*(b)).y;
  param_2 = vec2<f32>((x_112 - x_113), (x_115 - x_116));
  param_3 = vec2<f32>((x_120 - x_121), (x_124 - x_125));
  let x_129 : f32 = param_2.x;
  let x_131 : f32 = param_3.y;
  let x_134 : f32 = param_3.x;
  let x_136 : f32 = param_2.y;
  let x_138 : f32 = ((x_129 * x_131) - (x_134 * x_136));
  x_79 = x_138;
  let x_139 : bool = (x_111 < 0.0);
  let x_141 : bool = (x_139 & (x_138 < 0.0));
  x_148_phi = x_141;
  if (!(x_141)) {
    x_147 = ((x_111 >= 0.0) & (x_138 >= 0.0));
    x_148_phi = x_147;
  }
  var x_153_phi : i32;
  let x_148 : bool = x_148_phi;
  if (!(x_148)) {
    x_153_phi = 0;
    loop {
      var x_154 : i32;
      var x_164_phi : i32;
      let x_153 : i32 = x_153_phi;
      let x_159 : f32 = x_11.injectionSwitch.y;
      let x_160 : i32 = i32(x_159);
      if ((x_153 < x_160)) {
      } else {
        break;
      }
      x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
      x_164_phi = 0;
      loop {
        var x_165 : i32;
        let x_164 : i32 = x_164_phi;
        if ((x_164 < x_160)) {
        } else {
          break;
        }
        x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);

        continuing {
          x_165 = (x_164 + 1);
          x_164_phi = x_165;
        }
      }

      continuing {
        x_154 = (x_153 + 1);
        x_153_phi = x_154;
      }
    }
    return 0;
  }
  let x_171 : f32 = (*(p)).x;
  let x_172 : f32 = (*(c)).x;
  let x_174 : f32 = (*(p)).y;
  let x_175 : f32 = (*(c)).y;
  let x_178 : f32 = (*(a)).x;
  let x_179 : f32 = (*(c)).x;
  let x_181 : f32 = (*(a)).y;
  let x_182 : f32 = (*(c)).y;
  param_4 = vec2<f32>((x_171 - x_172), (x_174 - x_175));
  param_5 = vec2<f32>((x_178 - x_179), (x_181 - x_182));
  let x_186 : f32 = param_4.x;
  let x_188 : f32 = param_5.y;
  let x_191 : f32 = param_5.x;
  let x_193 : f32 = param_4.y;
  let x_195 : f32 = ((x_186 * x_188) - (x_191 * x_193));
  x_78 = x_195;
  let x_197 : bool = (x_139 & (x_195 < 0.0));
  x_204_phi = x_197;
  if (!(x_197)) {
    x_203 = ((x_111 >= 0.0) & (x_195 >= 0.0));
    x_204_phi = x_203;
  }
  let x_204 : bool = x_204_phi;
  if (!(x_204)) {
    return 0;
  }
  return 1;
}

fn main_1() {
  var param_6 : vec2<f32>;
  var param_7 : vec2<f32>;
  var param_8 : vec2<f32>;
  var param_9 : vec2<f32>;
  let x_60 : vec4<f32> = gl_FragCoord;
  let x_63 : vec2<f32> = x_19.resolution;
  param_6 = (vec2<f32>(x_60.x, x_60.y) / x_63);
  param_7 = vec2<f32>(0.699999988, 0.300000012);
  param_8 = vec2<f32>(0.5, 0.899999976);
  param_9 = vec2<f32>(0.100000001, 0.400000006);
  let x_65 : i32 = pointInTriangle_vf2_vf2_vf2_vf2_(&(param_6), &(param_7), &(param_8), &(param_9));
  if ((x_65 == 1)) {
    let x_71 : f32 = x_11.injectionSwitch.y;
    let x_73 : f32 = x_11.injectionSwitch.x;
    if ((x_71 >= x_73)) {
      x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    }
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 1.0);
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
