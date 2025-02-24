struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 18u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 4u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

struct buf2 {
  one : i32,
}

var<private> x_GLF_global_loop_count : i32;

@group(0) @binding(1) var<uniform> x_7 : buf1;

@group(0) @binding(0) var<uniform> x_12 : buf0;

@group(0) @binding(2) var<uniform> x_15 : buf2;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var m : mat4x4<f32>;
  var v : vec4<f32>;
  var f : f32;
  var a : i32;
  var b : i32;
  var zero : f32;
  x_GLF_global_loop_count = 0;
  let x_62 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  let x_64 : f32 = x_7.x_GLF_uniform_float_values[2].el;
  let x_66 : f32 = x_7.x_GLF_uniform_float_values[3].el;
  let x_68 : f32 = x_7.x_GLF_uniform_float_values[4].el;
  let x_70 : f32 = x_7.x_GLF_uniform_float_values[5].el;
  let x_72 : f32 = x_7.x_GLF_uniform_float_values[6].el;
  let x_74 : f32 = x_7.x_GLF_uniform_float_values[7].el;
  let x_76 : f32 = x_7.x_GLF_uniform_float_values[8].el;
  let x_78 : f32 = x_7.x_GLF_uniform_float_values[9].el;
  let x_80 : f32 = x_7.x_GLF_uniform_float_values[10].el;
  let x_82 : f32 = x_7.x_GLF_uniform_float_values[11].el;
  let x_84 : f32 = x_7.x_GLF_uniform_float_values[12].el;
  let x_86 : f32 = x_7.x_GLF_uniform_float_values[13].el;
  let x_88 : f32 = x_7.x_GLF_uniform_float_values[14].el;
  let x_90 : f32 = x_7.x_GLF_uniform_float_values[15].el;
  let x_92 : f32 = x_7.x_GLF_uniform_float_values[16].el;
  m = mat4x4<f32>(vec4<f32>(x_62, x_64, x_66, x_68), vec4<f32>(x_70, x_72, x_74, x_76), vec4<f32>(x_78, x_80, x_82, x_84), vec4<f32>(x_86, x_88, x_90, x_92));
  let x_99 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  let x_101 : f32 = x_7.x_GLF_uniform_float_values[2].el;
  let x_103 : f32 = x_7.x_GLF_uniform_float_values[3].el;
  let x_105 : f32 = x_7.x_GLF_uniform_float_values[4].el;
  v = vec4<f32>(x_99, x_101, x_103, x_105);
  let x_108 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  f = x_108;
  let x_110 : i32 = x_12.x_GLF_uniform_int_values[0].el;
  a = x_110;
  loop {
    let x_115 : i32 = x_GLF_global_loop_count;
    if ((x_115 < 10)) {
    } else {
      break;
    }
    let x_118 : i32 = x_GLF_global_loop_count;
    x_GLF_global_loop_count = (x_118 + 1);
    let x_120 : i32 = a;
    let x_121 : i32 = clamp(x_120, 0, 3);
    let x_123 : f32 = x_7.x_GLF_uniform_float_values[1].el;
    let x_125 : f32 = v[x_121];
    v[x_121] = (x_125 + x_123);
    let x_129 : i32 = x_12.x_GLF_uniform_int_values[2].el;
    b = x_129;
    loop {
      let x_134 : i32 = x_GLF_global_loop_count;
      if ((x_134 < 10)) {
      } else {
        break;
      }
      let x_137 : i32 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_137 + 1);
      let x_139 : i32 = b;
      let x_142 : f32 = v[clamp(x_139, 0, 3)];
      let x_143 : i32 = b;
      let x_145 : i32 = a;
      let x_147 : f32 = m[clamp(x_143, 0, 3)][x_145];
      let x_149 : f32 = f;
      f = (x_149 + (x_142 * x_147));

      continuing {
        let x_151 : i32 = b;
        b = (x_151 - 1);
      }
    }
    let x_153 : i32 = a;
    let x_156 : f32 = x_7.x_GLF_uniform_float_values[1].el;
    m[1][clamp(x_153, 0, 3)] = x_156;
    let x_159 : i32 = x_15.one;
    let x_161 : i32 = x_12.x_GLF_uniform_int_values[0].el;
    if ((x_159 == x_161)) {
      discard;
    }
    let x_166 : i32 = x_15.one;
    let x_168 : i32 = x_12.x_GLF_uniform_int_values[1].el;
    if ((x_166 == x_168)) {
      discard;
    }

    continuing {
      let x_172 : i32 = a;
      a = (x_172 + 1);
    }
  }
  let x_175 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  zero = x_175;
  let x_176 : f32 = f;
  let x_178 : f32 = x_7.x_GLF_uniform_float_values[17].el;
  if (!((x_176 == x_178))) {
    let x_183 : f32 = x_7.x_GLF_uniform_float_values[1].el;
    zero = x_183;
  }
  let x_184 : f32 = f;
  let x_185 : f32 = zero;
  let x_187 : i32 = x_12.x_GLF_uniform_int_values[0].el;
  let x_189 : f32 = f;
  x_GLF_color = vec4<f32>(x_184, x_185, f32(x_187), x_189);
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
