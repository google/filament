struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_20 : buf0;

var<private> x_GLF_color : vec4<f32>;

var<private> index : i32;

var<private> state : array<i32, 16u>;

fn collision_vf2_vf4_(pos : ptr<function, vec2<f32>>, quad : ptr<function, vec4<f32>>) -> bool {
  var x_116 : vec4<bool> = vec4<bool>(false, false, false, false);
  let x_118 : f32 = (*(pos)).x;
  let x_120 : f32 = (*(quad)).x;
  if ((x_118 < x_120)) {
    return false;
  }
  let x_125 : f32 = (*(pos)).y;
  let x_127 : f32 = (*(quad)).y;
  if ((x_125 < x_127)) {
    return false;
  }
  let x_132 : f32 = (*(pos)).x;
  let x_134 : f32 = (*(quad)).x;
  let x_136 : f32 = (*(quad)).z;
  if ((x_132 > (x_134 + x_136))) {
    return false;
  }
  let x_142 : f32 = (*(pos)).y;
  let x_144 : f32 = (*(quad)).y;
  let x_146 : f32 = (*(quad)).w;
  if ((x_142 > (x_144 + x_146))) {
    return false;
  }
  return true;
}

fn match_vf2_(pos_1 : ptr<function, vec2<f32>>) -> vec4<f32> {
  var res : vec4<f32>;
  var i : i32;
  var param : vec2<f32>;
  var param_1 : vec4<f32>;
  var indexable : array<vec4<f32>, 8u>;
  var indexable_1 : array<vec4<f32>, 8u>;
  var indexable_2 : array<vec4<f32>, 8u>;
  var indexable_3 : array<vec4<f32>, 16u>;
  res = vec4<f32>(0.5, 0.5, 1.0, 1.0);
  i = 0;
  loop {
    let x_156 : i32 = i;
    if ((x_156 < 8)) {
    } else {
      break;
    }
    let x_159 : i32 = i;
    let x_160 : vec2<f32> = *(pos_1);
    param = x_160;
    indexable = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
    let x_162 : vec4<f32> = indexable[x_159];
    param_1 = x_162;
    let x_163 : bool = collision_vf2_vf4_(&(param), &(param_1));
    if (x_163) {
      let x_166 : i32 = i;
      indexable_1 = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
      let x_168 : f32 = indexable_1[x_166].x;
      let x_170 : i32 = i;
      indexable_2 = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
      let x_172 : f32 = indexable_2[x_170].y;
      let x_175 : i32 = i;
      indexable_3 = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
      let x_181 : vec4<f32> = indexable_3[((((i32(x_168) * i32(x_172)) + (x_175 * 9)) + 11) % 16)];
      res = x_181;
    }

    continuing {
      let x_182 : i32 = i;
      i = (x_182 + 1);
    }
  }
  let x_184 : vec4<f32> = res;
  return x_184;
}

fn main_1() {
  var lin : vec2<f32>;
  var param_2 : vec2<f32>;
  let x_105 : vec4<f32> = gl_FragCoord;
  let x_108 : vec2<f32> = x_20.resolution;
  lin = (vec2<f32>(x_105.x, x_105.y) / x_108);
  let x_110 : vec2<f32> = lin;
  lin = floor((x_110 * 32.0));
  let x_113 : vec2<f32> = lin;
  param_2 = x_113;
  let x_114 : vec4<f32> = match_vf2_(&(param_2));
  x_GLF_color = x_114;
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
