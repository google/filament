struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_17 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn pointInTriangle_vf2_vf2_vf2_vf2_(p : ptr<function, vec2<f32>>, a : ptr<function, vec2<f32>>, b : ptr<function, vec2<f32>>, c : ptr<function, vec2<f32>>) -> i32 {
  var x_66 : f32;
  var x_67 : f32;
  var x_68 : f32;
  var param : vec2<f32>;
  var param_1 : vec2<f32>;
  var param_2 : vec2<f32>;
  var param_3 : vec2<f32>;
  var param_4 : vec2<f32>;
  var param_5 : vec2<f32>;
  var x_135 : bool;
  var x_172 : bool;
  var x_136_phi : bool;
  var x_173_phi : bool;
  let x_70 : f32 = (*(p)).x;
  let x_72 : f32 = (*(a)).x;
  let x_75 : f32 = (*(p)).y;
  let x_77 : f32 = (*(a)).y;
  let x_81 : f32 = (*(b)).x;
  let x_82 : f32 = (*(a)).x;
  let x_85 : f32 = (*(b)).y;
  let x_86 : f32 = (*(a)).y;
  param = vec2<f32>((x_70 - x_72), (x_75 - x_77));
  param_1 = vec2<f32>((x_81 - x_82), (x_85 - x_86));
  let x_90 : f32 = param.x;
  let x_92 : f32 = param_1.y;
  let x_95 : f32 = param_1.x;
  let x_97 : f32 = param.y;
  let x_99 : f32 = ((x_90 * x_92) - (x_95 * x_97));
  x_68 = x_99;
  let x_100 : f32 = (*(p)).x;
  let x_101 : f32 = (*(b)).x;
  let x_103 : f32 = (*(p)).y;
  let x_104 : f32 = (*(b)).y;
  let x_108 : f32 = (*(c)).x;
  let x_109 : f32 = (*(b)).x;
  let x_112 : f32 = (*(c)).y;
  let x_113 : f32 = (*(b)).y;
  param_2 = vec2<f32>((x_100 - x_101), (x_103 - x_104));
  param_3 = vec2<f32>((x_108 - x_109), (x_112 - x_113));
  let x_117 : f32 = param_2.x;
  let x_119 : f32 = param_3.y;
  let x_122 : f32 = param_3.x;
  let x_124 : f32 = param_2.y;
  let x_126 : f32 = ((x_117 * x_119) - (x_122 * x_124));
  x_67 = x_126;
  let x_127 : bool = (x_99 < 0.0);
  let x_129 : bool = (x_127 & (x_126 < 0.0));
  x_136_phi = x_129;
  if (!(x_129)) {
    x_135 = ((x_99 >= 0.0) & (x_126 >= 0.0));
    x_136_phi = x_135;
  }
  let x_136 : bool = x_136_phi;
  if (!(x_136)) {
    return 0;
  }
  let x_140 : f32 = (*(p)).x;
  let x_141 : f32 = (*(c)).x;
  let x_143 : f32 = (*(p)).y;
  let x_144 : f32 = (*(c)).y;
  let x_147 : f32 = (*(a)).x;
  let x_148 : f32 = (*(c)).x;
  let x_150 : f32 = (*(a)).y;
  let x_151 : f32 = (*(c)).y;
  param_4 = vec2<f32>((x_140 - x_141), (x_143 - x_144));
  param_5 = vec2<f32>((x_147 - x_148), (x_150 - x_151));
  let x_155 : f32 = param_4.x;
  let x_157 : f32 = param_5.y;
  let x_160 : f32 = param_5.x;
  let x_162 : f32 = param_4.y;
  let x_164 : f32 = ((x_155 * x_157) - (x_160 * x_162));
  x_66 = x_164;
  let x_166 : bool = (x_127 & (x_164 < 0.0));
  x_173_phi = x_166;
  if (!(x_166)) {
    x_172 = ((x_99 >= 0.0) & (x_164 >= 0.0));
    x_173_phi = x_172;
  }
  let x_173 : bool = x_173_phi;
  if (!(x_173)) {
    return 0;
  }
  return 1;
}

fn main_1() {
  var param_6 : vec2<f32>;
  var param_7 : vec2<f32>;
  var param_8 : vec2<f32>;
  var param_9 : vec2<f32>;
  let x_55 : vec4<f32> = gl_FragCoord;
  let x_58 : vec2<f32> = x_17.resolution;
  param_6 = (vec2<f32>(x_55.x, x_55.y) / x_58);
  param_7 = vec2<f32>(0.699999988, 0.300000012);
  param_8 = vec2<f32>(0.5, 0.899999976);
  param_9 = vec2<f32>(0.100000001, 0.400000006);
  let x_60 : i32 = pointInTriangle_vf2_vf2_vf2_vf2_(&(param_6), &(param_7), &(param_8), &(param_9));
  if ((x_60 == 1)) {
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
