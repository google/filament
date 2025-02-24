struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_20 : buf0;

var<private> x_GLF_color : vec4<f32>;

var<private> index : i32;

var<private> state : array<i32, 16u>;

fn collision_vf2_vf4_(pos : ptr<function, vec2<f32>>, quad : ptr<function, vec4<f32>>) -> bool {
  let x_114 : f32 = (*(pos)).x;
  let x_116 : f32 = (*(quad)).x;
  if ((x_114 < x_116)) {
    return false;
  }
  let x_121 : f32 = (*(pos)).y;
  let x_123 : f32 = (*(quad)).y;
  if ((x_121 < x_123)) {
    return false;
  }
  let x_128 : f32 = (*(pos)).x;
  let x_130 : f32 = (*(quad)).x;
  let x_132 : f32 = (*(quad)).z;
  if ((x_128 > (x_130 + x_132))) {
    return false;
  }
  let x_138 : f32 = (*(pos)).y;
  let x_140 : f32 = (*(quad)).y;
  let x_142 : f32 = (*(quad)).w;
  if ((x_138 > (x_140 + x_142))) {
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
    let x_152 : i32 = i;
    if ((x_152 < 8)) {
    } else {
      break;
    }
    let x_155 : i32 = i;
    let x_156 : vec2<f32> = *(pos_1);
    param = x_156;
    indexable = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
    let x_158 : vec4<f32> = indexable[x_155];
    param_1 = x_158;
    let x_159 : bool = collision_vf2_vf4_(&(param), &(param_1));
    if (x_159) {
      let x_162 : i32 = i;
      indexable_1 = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
      let x_164 : f32 = indexable_1[x_162].x;
      let x_166 : i32 = i;
      indexable_2 = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
      let x_168 : f32 = indexable_2[x_166].y;
      let x_171 : i32 = i;
      indexable_3 = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
      let x_177 : vec4<f32> = indexable_3[((((i32(x_164) * i32(x_168)) + (x_171 * 9)) + 11) % 16)];
      res = x_177;
    }

    continuing {
      let x_178 : i32 = i;
      i = (x_178 + 1);
    }
  }
  let x_180 : vec4<f32> = res;
  return x_180;
}

fn main_1() {
  var lin : vec2<f32>;
  var param_2 : vec2<f32>;
  let x_102 : vec4<f32> = gl_FragCoord;
  let x_105 : vec2<f32> = x_20.resolution;
  lin = (vec2<f32>(x_102.x, x_102.y) / x_105);
  let x_107 : vec2<f32> = lin;
  lin = floor((x_107 * 32.0));
  let x_110 : vec2<f32> = lin;
  param_2 = x_110;
  let x_111 : vec4<f32> = match_vf2_(&(param_2));
  x_GLF_color = x_111;
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
