struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 4u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(1) var<uniform> x_9 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var v : vec4<f32>;
  var m : mat3x4<f32>;
  var indexable : mat4x4<f32>;
  let x_44 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  a = x_44;
  let x_46 : f32 = x_9.x_GLF_uniform_float_values[2].el;
  v = vec4<f32>(x_46, x_46, x_46, x_46);
  let x_49 : f32 = x_9.x_GLF_uniform_float_values[3].el;
  m = mat3x4<f32>(vec4<f32>(x_49, 0.0, 0.0, 0.0), vec4<f32>(0.0, x_49, 0.0, 0.0), vec4<f32>(0.0, 0.0, x_49, 0.0));
  let x_54 : i32 = a;
  let x_55 : i32 = a;
  let x_57 : f32 = x_9.x_GLF_uniform_float_values[0].el;
  m[x_54][x_55] = x_57;
  let x_59 : i32 = a;
  let x_60 : mat3x4<f32> = m;
  let x_78 : i32 = a;
  let x_79 : i32 = a;
  indexable = mat4x4<f32>(vec4<f32>(x_60[0u].x, x_60[0u].y, x_60[0u].z, x_60[0u].w), vec4<f32>(x_60[1u].x, x_60[1u].y, x_60[1u].z, x_60[1u].w), vec4<f32>(x_60[2u].x, x_60[2u].y, x_60[2u].z, x_60[2u].w), vec4<f32>(0.0, 0.0, 0.0, 1.0));
  let x_81 : f32 = indexable[x_78][x_79];
  let x_83 : f32 = v[x_59];
  v[x_59] = (x_83 + x_81);
  let x_87 : f32 = v.y;
  let x_89 : f32 = x_9.x_GLF_uniform_float_values[1].el;
  if ((x_87 == x_89)) {
    let x_95 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_98 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_101 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_104 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_95), f32(x_98), f32(x_101), f32(x_104));
  } else {
    let x_108 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_109 : f32 = f32(x_108);
    x_GLF_color = vec4<f32>(x_109, x_109, x_109, x_109);
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
