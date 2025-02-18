struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn collatz_i1_(v : ptr<function, i32>) -> i32 {
  var count : i32;
  count = 0;
  loop {
    let x_89 : i32 = *(v);
    if ((x_89 > 1)) {
    } else {
      break;
    }
    let x_92 : i32 = *(v);
    if (((x_92 & 1) == 1)) {
      let x_98 : i32 = *(v);
      *(v) = ((3 * x_98) + 1);
    } else {
      let x_101 : i32 = *(v);
      *(v) = (x_101 / 2);
    }
    let x_103 : i32 = count;
    count = (x_103 + 1);
  }
  let x_105 : i32 = count;
  return x_105;
}

fn main_1() {
  var lin : vec2<f32>;
  var v_1 : i32;
  var param : i32;
  var indexable : array<vec4<f32>, 16u>;
  let x_63 : vec4<f32> = gl_FragCoord;
  let x_66 : vec2<f32> = x_10.resolution;
  lin = (vec2<f32>(x_63.x, x_63.y) / x_66);
  let x_68 : vec2<f32> = lin;
  lin = floor((x_68 * 8.0));
  let x_72 : f32 = lin.x;
  let x_76 : f32 = lin.y;
  v_1 = ((i32(x_72) * 8) + i32(x_76));
  let x_79 : i32 = v_1;
  param = x_79;
  let x_80 : i32 = collatz_i1_(&(param));
  indexable = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
  let x_83 : vec4<f32> = indexable[(x_80 % 16)];
  x_GLF_color = x_83;
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
