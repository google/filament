struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_24 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn cross2d_vf2_vf2_(a : ptr<function, vec2<f32>>, b : ptr<function, vec2<f32>>) -> f32 {
  let x_79 : f32 = (*(a)).x;
  let x_81 : f32 = (*(b)).y;
  let x_84 : f32 = (*(b)).x;
  let x_86 : f32 = (*(a)).y;
  return ((x_79 * x_81) - (x_84 * x_86));
}

fn pointInTriangle_vf2_vf2_vf2_vf2_(p : ptr<function, vec2<f32>>, a_1 : ptr<function, vec2<f32>>, b_1 : ptr<function, vec2<f32>>, c : ptr<function, vec2<f32>>) -> i32 {
  var x_90 : bool = false;
  var x_91 : i32;
  var pab : f32;
  var param : vec2<f32>;
  var param_1 : vec2<f32>;
  var pbc : f32;
  var param_2 : vec2<f32>;
  var param_3 : vec2<f32>;
  var pca : f32;
  var param_4 : vec2<f32>;
  var param_5 : vec2<f32>;
  var x_140 : bool;
  var x_168 : bool;
  var x_141_phi : bool;
  var x_169_phi : bool;
  var x_173_phi : i32;
  switch(0u) {
    default: {
      let x_95 : f32 = (*(p)).x;
      let x_97 : f32 = (*(a_1)).x;
      let x_100 : f32 = (*(p)).y;
      let x_102 : f32 = (*(a_1)).y;
      let x_106 : f32 = (*(b_1)).x;
      let x_107 : f32 = (*(a_1)).x;
      let x_110 : f32 = (*(b_1)).y;
      let x_111 : f32 = (*(a_1)).y;
      param = vec2<f32>((x_95 - x_97), (x_100 - x_102));
      param_1 = vec2<f32>((x_106 - x_107), (x_110 - x_111));
      let x_114 : f32 = cross2d_vf2_vf2_(&(param), &(param_1));
      pab = x_114;
      let x_115 : f32 = (*(p)).x;
      let x_116 : f32 = (*(b_1)).x;
      let x_118 : f32 = (*(p)).y;
      let x_119 : f32 = (*(b_1)).y;
      let x_123 : f32 = (*(c)).x;
      let x_124 : f32 = (*(b_1)).x;
      let x_127 : f32 = (*(c)).y;
      let x_128 : f32 = (*(b_1)).y;
      param_2 = vec2<f32>((x_115 - x_116), (x_118 - x_119));
      param_3 = vec2<f32>((x_123 - x_124), (x_127 - x_128));
      let x_131 : f32 = cross2d_vf2_vf2_(&(param_2), &(param_3));
      pbc = x_131;
      let x_134 : bool = ((x_114 < 0.0) & (x_131 < 0.0));
      x_141_phi = x_134;
      if (!(x_134)) {
        x_140 = ((x_114 >= 0.0) & (x_131 >= 0.0));
        x_141_phi = x_140;
      }
      let x_141 : bool = x_141_phi;
      if (!(x_141)) {
        x_90 = true;
        x_91 = 0;
        x_173_phi = 0;
        break;
      }
      let x_145 : f32 = (*(p)).x;
      let x_146 : f32 = (*(c)).x;
      let x_148 : f32 = (*(p)).y;
      let x_149 : f32 = (*(c)).y;
      let x_152 : f32 = (*(a_1)).x;
      let x_153 : f32 = (*(c)).x;
      let x_155 : f32 = (*(a_1)).y;
      let x_156 : f32 = (*(c)).y;
      param_4 = vec2<f32>((x_145 - x_146), (x_148 - x_149));
      param_5 = vec2<f32>((x_152 - x_153), (x_155 - x_156));
      let x_159 : f32 = cross2d_vf2_vf2_(&(param_4), &(param_5));
      pca = x_159;
      let x_162 : bool = ((x_114 < 0.0) & (x_159 < 0.0));
      x_169_phi = x_162;
      if (!(x_162)) {
        x_168 = ((x_114 >= 0.0) & (x_159 >= 0.0));
        x_169_phi = x_168;
      }
      let x_169 : bool = x_169_phi;
      if (!(x_169)) {
        x_90 = true;
        x_91 = 0;
        x_173_phi = 0;
        break;
      }
      x_90 = true;
      x_91 = 1;
      x_173_phi = 1;
    }
  }
  let x_173 : i32 = x_173_phi;
  return x_173;
}

fn main_1() {
  var pos : vec2<f32>;
  var param_6 : vec2<f32>;
  var param_7 : vec2<f32>;
  var param_8 : vec2<f32>;
  var param_9 : vec2<f32>;
  let x_67 : vec4<f32> = gl_FragCoord;
  let x_70 : vec2<f32> = x_24.resolution;
  let x_71 : vec2<f32> = (vec2<f32>(x_67.x, x_67.y) / x_70);
  pos = x_71;
  param_6 = x_71;
  param_7 = vec2<f32>(0.699999988, 0.300000012);
  param_8 = vec2<f32>(0.5, 0.899999976);
  param_9 = vec2<f32>(0.100000001, 0.400000006);
  let x_72 : i32 = pointInTriangle_vf2_vf2_vf2_vf2_(&(param_6), &(param_7), &(param_8), &(param_9));
  if ((x_72 == 1)) {
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
