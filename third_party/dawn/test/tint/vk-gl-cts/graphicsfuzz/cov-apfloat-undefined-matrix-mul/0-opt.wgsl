struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 12u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_15 : buf0;

fn main_1() {
  var m0 : mat3x4<f32>;
  var m1 : mat3x4<f32>;
  var undefined : vec3<f32>;
  var defined : vec3<f32>;
  var v0 : vec4<f32>;
  var v1 : vec4<f32>;
  var v2 : vec4<f32>;
  var v3 : vec4<f32>;
  let x_17 : i32 = x_6.x_GLF_uniform_int_values[4].el;
  let x_18 : i32 = x_6.x_GLF_uniform_int_values[5].el;
  let x_19 : i32 = x_6.x_GLF_uniform_int_values[6].el;
  let x_20 : i32 = x_6.x_GLF_uniform_int_values[10].el;
  let x_21 : i32 = x_6.x_GLF_uniform_int_values[7].el;
  let x_22 : i32 = x_6.x_GLF_uniform_int_values[8].el;
  let x_23 : i32 = x_6.x_GLF_uniform_int_values[11].el;
  let x_24 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_25 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_26 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  m0 = mat3x4<f32>(vec4<f32>(f32(x_17), f32(x_18), f32(x_19), 4.0), vec4<f32>(f32(x_20), f32(x_21), f32(x_22), 8.0), vec4<f32>(f32(x_23), f32(x_24), f32(x_25), f32(x_26)));
  let x_27 : i32 = x_6.x_GLF_uniform_int_values[4].el;
  let x_104 : f32 = f32(x_27);
  m1 = mat3x4<f32>(vec4<f32>(x_104, 0.0, 0.0, 0.0), vec4<f32>(0.0, x_104, 0.0, 0.0), vec4<f32>(0.0, 0.0, x_104, 0.0));
  undefined = ldexp(vec3<f32>(1.0, 1.0, 1.0), vec3<i32>(1, 1, 1));
  let x_28 : i32 = x_6.x_GLF_uniform_int_values[4].el;
  let x_111 : f32 = f32(x_28);
  let x_29 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  defined = ldexp(vec3<f32>(x_111, x_111, x_111), vec3<i32>(x_29, x_29, x_29));
  let x_116 : mat3x4<f32> = m0;
  let x_117 : vec3<f32> = undefined;
  v0 = (x_116 * x_117);
  let x_119 : mat3x4<f32> = m1;
  let x_120 : vec3<f32> = undefined;
  v1 = (x_119 * x_120);
  let x_122 : mat3x4<f32> = m0;
  let x_123 : vec3<f32> = defined;
  v2 = (x_122 * x_123);
  let x_125 : mat3x4<f32> = m1;
  let x_126 : vec3<f32> = defined;
  v3 = (x_125 * x_126);
  let x_129 : f32 = v2.x;
  let x_131 : f32 = v3.x;
  if ((x_129 > x_131)) {
    let x_30 : i32 = x_6.x_GLF_uniform_int_values[4].el;
    let x_31 : i32 = x_6.x_GLF_uniform_int_values[9].el;
    let x_32 : i32 = x_6.x_GLF_uniform_int_values[9].el;
    let x_33 : i32 = x_6.x_GLF_uniform_int_values[4].el;
    x_GLF_color = vec4<f32>(f32(x_30), f32(x_31), f32(x_32), f32(x_33));
  } else {
    let x_34 : i32 = x_6.x_GLF_uniform_int_values[9].el;
    let x_146 : f32 = f32(x_34);
    x_GLF_color = vec4<f32>(x_146, x_146, x_146, x_146);
  }
  let x_149 : f32 = v0.x;
  let x_151 : f32 = v1.x;
  if ((x_149 < x_151)) {
    let x_156 : f32 = x_15.x_GLF_uniform_float_values[0].el;
    x_GLF_color.y = x_156;
  }
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
