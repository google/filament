struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

@group(0) @binding(1) var<uniform> x_9 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn f1_vf2_(v1 : ptr<function, vec2<f32>>) -> i32 {
  var x_99 : bool;
  var x_100_phi : bool;
  let x_89 : f32 = (*(v1)).x;
  let x_91 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  let x_92 : bool = (x_89 == x_91);
  x_100_phi = x_92;
  if (x_92) {
    let x_96 : f32 = (*(v1)).y;
    let x_98 : f32 = x_7.x_GLF_uniform_float_values[1].el;
    x_99 = (x_96 == x_98);
    x_100_phi = x_99;
  }
  let x_100 : bool = x_100_phi;
  if (x_100) {
    let x_104 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    return x_104;
  }
  let x_106 : i32 = x_9.x_GLF_uniform_int_values[0].el;
  return x_106;
}

fn main_1() {
  var m1 : mat2x2<f32>;
  var m2 : mat2x2<f32>;
  var v1_1 : vec2<f32>;
  var a : i32;
  var param : vec2<f32>;
  let x_45 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  let x_47 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  let x_50 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  let x_52 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  m1 = mat2x2<f32>(vec2<f32>(x_45, -(x_47)), vec2<f32>(x_50, sin(x_52)));
  let x_57 : mat2x2<f32> = m1;
  let x_58 : mat2x2<f32> = m1;
  m2 = (x_57 * x_58);
  let x_61 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  let x_63 : mat2x2<f32> = m2;
  v1_1 = (vec2<f32>(x_61, x_61) * x_63);
  let x_65 : vec2<f32> = v1_1;
  param = x_65;
  let x_66 : i32 = f1_vf2_(&(param));
  a = x_66;
  let x_67 : i32 = a;
  let x_69 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  if ((x_67 == x_69)) {
    let x_75 : f32 = x_7.x_GLF_uniform_float_values[0].el;
    let x_77 : f32 = x_7.x_GLF_uniform_float_values[1].el;
    let x_79 : f32 = x_7.x_GLF_uniform_float_values[1].el;
    let x_81 : f32 = x_7.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4<f32>(x_75, x_77, x_79, x_81);
  } else {
    let x_84 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_85 : f32 = f32(x_84);
    x_GLF_color = vec4<f32>(x_85, x_85, x_85, x_85);
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
