struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_20 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn collision_vf2_vf4_(pos : ptr<function, vec2<f32>>, quad : ptr<function, vec4<f32>>) -> bool {
  let x_110 : f32 = (*(pos)).x;
  let x_112 : f32 = (*(quad)).x;
  if ((x_110 < x_112)) {
    return false;
  }
  let x_117 : f32 = (*(pos)).y;
  let x_119 : f32 = (*(quad)).y;
  if ((x_117 < x_119)) {
    return false;
  }
  let x_124 : f32 = (*(pos)).x;
  let x_126 : f32 = (*(quad)).x;
  let x_128 : f32 = (*(quad)).z;
  if ((x_124 > (x_126 + x_128))) {
    return false;
  }
  let x_134 : f32 = (*(pos)).y;
  let x_136 : f32 = (*(quad)).y;
  let x_138 : f32 = (*(quad)).w;
  if ((x_134 > (x_136 + x_138))) {
    return false;
  }
  return true;
}

fn match_vf2_(pos_1 : ptr<function, vec2<f32>>) -> vec4<f32> {
  var res : vec4<f32>;
  var x_144 : f32;
  var x_145 : f32;
  var i : i32;
  var param : vec2<f32>;
  var param_1 : vec4<f32>;
  var indexable : array<vec4<f32>, 8u>;
  var indexable_1 : array<vec4<f32>, 8u>;
  var indexable_2 : array<vec4<f32>, 8u>;
  var indexable_3 : array<vec4<f32>, 16u>;
  let x_147 : f32 = gl_FragCoord.x;
  if ((x_147 < 0.0)) {
    x_144 = -1.0;
  } else {
    let x_153 : f32 = gl_FragCoord.x;
    if ((x_153 >= 0.0)) {
      let x_159 : f32 = gl_FragCoord.x;
      x_145 = select(1.0, 0.5, (x_159 >= 0.0));
    } else {
      x_145 = 1.0;
    }
    let x_162 : f32 = x_145;
    x_144 = min(x_162, 0.5);
  }
  let x_164 : f32 = x_144;
  res = vec4<f32>(clamp(0.5, 0.5, x_164), 0.5, 1.0, 1.0);
  i = 0;
  loop {
    let x_171 : i32 = i;
    if ((x_171 < 8)) {
    } else {
      break;
    }
    let x_174 : i32 = i;
    let x_175 : vec2<f32> = *(pos_1);
    param = x_175;
    indexable = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
    let x_177 : vec4<f32> = indexable[x_174];
    param_1 = x_177;
    let x_178 : bool = collision_vf2_vf4_(&(param), &(param_1));
    if (x_178) {
      let x_181 : i32 = i;
      indexable_1 = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
      let x_183 : f32 = indexable_1[x_181].x;
      let x_185 : i32 = i;
      indexable_2 = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
      let x_187 : f32 = indexable_2[x_185].y;
      let x_190 : i32 = i;
      indexable_3 = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
      let x_196 : vec4<f32> = indexable_3[((((i32(x_183) * i32(x_187)) + (x_190 * 9)) + 11) % 16)];
      res = x_196;
    }

    continuing {
      let x_197 : i32 = i;
      i = (x_197 + 1);
    }
  }
  let x_199 : vec4<f32> = res;
  return x_199;
}

fn main_1() {
  var lin : vec2<f32>;
  var param_2 : vec2<f32>;
  let x_98 : vec4<f32> = gl_FragCoord;
  let x_101 : vec2<f32> = x_20.resolution;
  lin = (vec2<f32>(x_98.x, x_98.y) / x_101);
  let x_103 : vec2<f32> = lin;
  lin = floor((x_103 * 32.0));
  let x_106 : vec2<f32> = lin;
  param_2 = x_106;
  let x_107 : vec4<f32> = match_vf2_(&(param_2));
  x_GLF_color = x_107;
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
