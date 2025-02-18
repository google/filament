struct buf0 {
  resolution : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_15 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn cross2d_vf2_vf2_(a : ptr<function, vec2<f32>>, b : ptr<function, vec2<f32>>) -> f32 {
  let x_85 : f32 = (*(a)).x;
  let x_87 : f32 = (*(b)).y;
  let x_90 : f32 = (*(b)).x;
  let x_92 : f32 = (*(a)).y;
  return ((x_85 * x_87) - (x_90 * x_92));
}

fn pointInTriangle_vf2_vf2_vf2_vf2_(p : ptr<function, vec2<f32>>, a_1 : ptr<function, vec2<f32>>, b_1 : ptr<function, vec2<f32>>, c : ptr<function, vec2<f32>>) -> i32 {
  var var_y : f32;
  var x_96 : f32;
  var x_97 : f32;
  var clamp_y : f32;
  var pab : f32;
  var param : vec2<f32>;
  var param_1 : vec2<f32>;
  var pbc : f32;
  var param_2 : vec2<f32>;
  var param_3 : vec2<f32>;
  var pca : f32;
  var param_4 : vec2<f32>;
  var param_5 : vec2<f32>;
  var x_173 : bool;
  var x_205 : bool;
  var x_174_phi : bool;
  var x_206_phi : bool;
  let x_99 : f32 = x_15.resolution.x;
  let x_101 : f32 = x_15.resolution.y;
  if ((x_99 == x_101)) {
    let x_107 : f32 = (*(c)).y;
    let x_108 : vec2<f32> = vec2<f32>(0.0, x_107);
    if (true) {
      let x_112 : f32 = (*(c)).y;
      x_97 = x_112;
    } else {
      x_97 = 1.0;
    }
    let x_113 : f32 = x_97;
    let x_114 : f32 = (*(c)).y;
    let x_116 : vec2<f32> = vec2<f32>(1.0, max(x_113, x_114));
    let x_117 : vec2<f32> = vec2<f32>(x_108.x, x_108.y);
    x_96 = x_107;
  } else {
    x_96 = -1.0;
  }
  let x_118 : f32 = x_96;
  var_y = x_118;
  let x_120 : f32 = (*(c)).y;
  let x_121 : f32 = (*(c)).y;
  let x_122 : f32 = var_y;
  clamp_y = clamp(x_120, x_121, x_122);
  let x_125 : f32 = (*(p)).x;
  let x_127 : f32 = (*(a_1)).x;
  let x_130 : f32 = (*(p)).y;
  let x_132 : f32 = (*(a_1)).y;
  let x_136 : f32 = (*(b_1)).x;
  let x_137 : f32 = (*(a_1)).x;
  let x_140 : f32 = (*(b_1)).y;
  let x_141 : f32 = (*(a_1)).y;
  param = vec2<f32>((x_125 - x_127), (x_130 - x_132));
  param_1 = vec2<f32>((x_136 - x_137), (x_140 - x_141));
  let x_144 : f32 = cross2d_vf2_vf2_(&(param), &(param_1));
  pab = x_144;
  let x_145 : f32 = (*(p)).x;
  let x_146 : f32 = (*(b_1)).x;
  let x_148 : f32 = (*(p)).y;
  let x_149 : f32 = (*(b_1)).y;
  let x_153 : f32 = (*(c)).x;
  let x_154 : f32 = (*(b_1)).x;
  let x_156 : f32 = clamp_y;
  let x_157 : f32 = (*(b_1)).y;
  param_2 = vec2<f32>((x_145 - x_146), (x_148 - x_149));
  param_3 = vec2<f32>((x_153 - x_154), (x_156 - x_157));
  let x_160 : f32 = cross2d_vf2_vf2_(&(param_2), &(param_3));
  pbc = x_160;
  let x_161 : f32 = pab;
  let x_163 : f32 = pbc;
  let x_165 : bool = ((x_161 < 0.0) & (x_163 < 0.0));
  x_174_phi = x_165;
  if (!(x_165)) {
    let x_169 : f32 = pab;
    let x_171 : f32 = pbc;
    x_173 = ((x_169 >= 0.0) & (x_171 >= 0.0));
    x_174_phi = x_173;
  }
  let x_174 : bool = x_174_phi;
  if (!(x_174)) {
    return 0;
  }
  let x_178 : f32 = (*(p)).x;
  let x_179 : f32 = (*(c)).x;
  let x_181 : f32 = (*(p)).y;
  let x_182 : f32 = (*(c)).y;
  let x_185 : f32 = (*(a_1)).x;
  let x_186 : f32 = (*(c)).x;
  let x_188 : f32 = (*(a_1)).y;
  let x_189 : f32 = (*(c)).y;
  param_4 = vec2<f32>((x_178 - x_179), (x_181 - x_182));
  param_5 = vec2<f32>((x_185 - x_186), (x_188 - x_189));
  let x_192 : f32 = cross2d_vf2_vf2_(&(param_4), &(param_5));
  pca = x_192;
  let x_193 : f32 = pab;
  let x_195 : f32 = pca;
  let x_197 : bool = ((x_193 < 0.0) & (x_195 < 0.0));
  x_206_phi = x_197;
  if (!(x_197)) {
    let x_201 : f32 = pab;
    let x_203 : f32 = pca;
    x_205 = ((x_201 >= 0.0) & (x_203 >= 0.0));
    x_206_phi = x_205;
  }
  let x_206 : bool = x_206_phi;
  if (!(x_206)) {
    return 0;
  }
  return 1;
}

fn main_1() {
  var pos : vec2<f32>;
  var param_6 : vec2<f32>;
  var param_7 : vec2<f32>;
  var param_8 : vec2<f32>;
  var param_9 : vec2<f32>;
  let x_72 : vec4<f32> = gl_FragCoord;
  let x_75 : vec2<f32> = x_15.resolution;
  pos = (vec2<f32>(x_72.x, x_72.y) / x_75);
  let x_77 : vec2<f32> = pos;
  param_6 = x_77;
  param_7 = vec2<f32>(0.699999988, 0.300000012);
  param_8 = vec2<f32>(0.5, 0.899999976);
  param_9 = vec2<f32>(0.100000001, 0.400000006);
  let x_78 : i32 = pointInTriangle_vf2_vf2_vf2_vf2_(&(param_6), &(param_7), &(param_8), &(param_9));
  if ((x_78 == 1)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
