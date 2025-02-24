struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_24 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn cross2d_vf2_vf2_(a : ptr<function, vec2<f32>>, b : ptr<function, vec2<f32>>) -> f32 {
  let x_76 : f32 = (*(a)).x;
  let x_78 : f32 = (*(b)).y;
  let x_81 : f32 = (*(b)).x;
  let x_83 : f32 = (*(a)).y;
  return ((x_76 * x_78) - (x_81 * x_83));
}

fn pointInTriangle_vf2_vf2_vf2_vf2_(p : ptr<function, vec2<f32>>, a_1 : ptr<function, vec2<f32>>, b_1 : ptr<function, vec2<f32>>, c : ptr<function, vec2<f32>>) -> i32 {
  var pab : f32;
  var param : vec2<f32>;
  var param_1 : vec2<f32>;
  var pbc : f32;
  var param_2 : vec2<f32>;
  var param_3 : vec2<f32>;
  var pca : f32;
  var param_4 : vec2<f32>;
  var param_5 : vec2<f32>;
  var x_137 : bool;
  var x_169 : bool;
  var x_138_phi : bool;
  var x_170_phi : bool;
  let x_88 : f32 = (*(p)).x;
  let x_90 : f32 = (*(a_1)).x;
  let x_93 : f32 = (*(p)).y;
  let x_95 : f32 = (*(a_1)).y;
  let x_99 : f32 = (*(b_1)).x;
  let x_100 : f32 = (*(a_1)).x;
  let x_103 : f32 = (*(b_1)).y;
  let x_104 : f32 = (*(a_1)).y;
  param = vec2<f32>((x_88 - x_90), (x_93 - x_95));
  param_1 = vec2<f32>((x_99 - x_100), (x_103 - x_104));
  let x_107 : f32 = cross2d_vf2_vf2_(&(param), &(param_1));
  pab = x_107;
  let x_108 : f32 = (*(p)).x;
  let x_109 : f32 = (*(b_1)).x;
  let x_111 : f32 = (*(p)).y;
  let x_112 : f32 = (*(b_1)).y;
  let x_116 : f32 = (*(c)).x;
  let x_117 : f32 = (*(b_1)).x;
  let x_120 : f32 = (*(c)).y;
  let x_121 : f32 = (*(b_1)).y;
  param_2 = vec2<f32>((x_108 - x_109), (x_111 - x_112));
  param_3 = vec2<f32>((x_116 - x_117), (x_120 - x_121));
  let x_124 : f32 = cross2d_vf2_vf2_(&(param_2), &(param_3));
  pbc = x_124;
  let x_125 : f32 = pab;
  let x_127 : f32 = pbc;
  let x_129 : bool = ((x_125 < 0.0) & (x_127 < 0.0));
  x_138_phi = x_129;
  if (!(x_129)) {
    let x_133 : f32 = pab;
    let x_135 : f32 = pbc;
    x_137 = ((x_133 >= 0.0) & (x_135 >= 0.0));
    x_138_phi = x_137;
  }
  let x_138 : bool = x_138_phi;
  if (!(x_138)) {
    return 0;
  }
  let x_142 : f32 = (*(p)).x;
  let x_143 : f32 = (*(c)).x;
  let x_145 : f32 = (*(p)).y;
  let x_146 : f32 = (*(c)).y;
  let x_149 : f32 = (*(a_1)).x;
  let x_150 : f32 = (*(c)).x;
  let x_152 : f32 = (*(a_1)).y;
  let x_153 : f32 = (*(c)).y;
  param_4 = vec2<f32>((x_142 - x_143), (x_145 - x_146));
  param_5 = vec2<f32>((x_149 - x_150), (x_152 - x_153));
  let x_156 : f32 = cross2d_vf2_vf2_(&(param_4), &(param_5));
  pca = x_156;
  let x_157 : f32 = pab;
  let x_159 : f32 = pca;
  let x_161 : bool = ((x_157 < 0.0) & (x_159 < 0.0));
  x_170_phi = x_161;
  if (!(x_161)) {
    let x_165 : f32 = pab;
    let x_167 : f32 = pca;
    x_169 = ((x_165 >= 0.0) & (x_167 >= 0.0));
    x_170_phi = x_169;
  }
  let x_170 : bool = x_170_phi;
  if (!(x_170)) {
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
  let x_63 : vec4<f32> = gl_FragCoord;
  let x_66 : vec2<f32> = x_24.resolution;
  pos = (vec2<f32>(x_63.x, x_63.y) / x_66);
  let x_68 : vec2<f32> = pos;
  param_6 = x_68;
  param_7 = vec2<f32>(0.699999988, 0.300000012);
  param_8 = vec2<f32>(0.5, 0.899999976);
  param_9 = vec2<f32>(0.100000001, 0.400000006);
  let x_69 : i32 = pointInTriangle_vf2_vf2_vf2_vf2_(&(param_6), &(param_7), &(param_8), &(param_9));
  if ((x_69 == 1)) {
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
