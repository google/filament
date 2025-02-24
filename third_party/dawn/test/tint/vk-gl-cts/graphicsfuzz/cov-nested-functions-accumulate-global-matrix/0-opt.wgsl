struct buf2 {
  one : f32,
}

struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 1u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 4u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

var<private> m : mat4x2<f32>;

@group(0) @binding(2) var<uniform> x_10 : buf2;

@group(0) @binding(0) var<uniform> x_12 : buf0;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_16 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn func0_i1_(x : ptr<function, i32>) {
  var i : i32;
  var x_137 : bool;
  var x_138 : bool;
  var x_138_phi : bool;
  var x_139_phi : bool;
  let x_124 : i32 = *(x);
  let x_125 : bool = (x_124 < 1);
  x_139_phi = x_125;
  if (!(x_125)) {
    let x_129 : i32 = *(x);
    let x_130 : bool = (x_129 > 1);
    x_138_phi = x_130;
    if (x_130) {
      let x_134 : f32 = x_10.one;
      let x_136 : f32 = x_12.x_GLF_uniform_float_values[0].el;
      x_137 = (x_134 > x_136);
      x_138_phi = x_137;
    }
    x_138 = x_138_phi;
    x_139_phi = x_138;
  }
  let x_139 : bool = x_139_phi;
  if (x_139) {
    return;
  }
  let x_143 : f32 = x_10.one;
  let x_145 : f32 = x_12.x_GLF_uniform_float_values[0].el;
  if ((x_143 == x_145)) {
    i = 0;
    loop {
      let x_150 : i32 = i;
      if ((x_150 < 2)) {
      } else {
        break;
      }

      continuing {
        let x_154 : i32 = *(x);
        let x_155 : i32 = clamp(x_154, 0, 3);
        let x_156 : i32 = i;
        let x_158 : f32 = x_12.x_GLF_uniform_float_values[0].el;
        let x_160 : f32 = m[x_155][x_156];
        m[x_155][x_156] = (x_160 + x_158);
        let x_163 : i32 = i;
        i = (x_163 + 1);
      }
    }
  }
  return;
}

fn func1_() {
  var param : i32;
  let x_167 : f32 = gl_FragCoord.y;
  if ((x_167 < 0.0)) {
    return;
  }
  param = 1;
  func0_i1_(&(param));
  return;
}

fn main_1() {
  m = mat4x2<f32>(vec2<f32>(0.0, 0.0), vec2<f32>(0.0, 0.0), vec2<f32>(0.0, 0.0), vec2<f32>(0.0, 0.0));
  func1_();
  func1_();
  let x_54 : mat4x2<f32> = m;
  let x_56 : i32 = x_16.x_GLF_uniform_int_values[0].el;
  let x_59 : i32 = x_16.x_GLF_uniform_int_values[0].el;
  let x_62 : i32 = x_16.x_GLF_uniform_int_values[1].el;
  let x_65 : i32 = x_16.x_GLF_uniform_int_values[1].el;
  let x_68 : i32 = x_16.x_GLF_uniform_int_values[0].el;
  let x_71 : i32 = x_16.x_GLF_uniform_int_values[0].el;
  let x_74 : i32 = x_16.x_GLF_uniform_int_values[0].el;
  let x_77 : i32 = x_16.x_GLF_uniform_int_values[0].el;
  let x_83 : mat4x2<f32> = mat4x2<f32>(vec2<f32>(f32(x_56), f32(x_59)), vec2<f32>(f32(x_62), f32(x_65)), vec2<f32>(f32(x_68), f32(x_71)), vec2<f32>(f32(x_74), f32(x_77)));
  if ((((all((x_54[0u] == x_83[0u])) & all((x_54[1u] == x_83[1u]))) & all((x_54[2u] == x_83[2u]))) & all((x_54[3u] == x_83[3u])))) {
    let x_107 : i32 = x_16.x_GLF_uniform_int_values[3].el;
    let x_110 : i32 = x_16.x_GLF_uniform_int_values[0].el;
    let x_113 : i32 = x_16.x_GLF_uniform_int_values[0].el;
    let x_116 : i32 = x_16.x_GLF_uniform_int_values[3].el;
    x_GLF_color = vec4<f32>(f32(x_107), f32(x_110), f32(x_113), f32(x_116));
  } else {
    let x_120 : i32 = x_16.x_GLF_uniform_int_values[0].el;
    let x_121 : f32 = f32(x_120);
    x_GLF_color = vec4<f32>(x_121, x_121, x_121, x_121);
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
