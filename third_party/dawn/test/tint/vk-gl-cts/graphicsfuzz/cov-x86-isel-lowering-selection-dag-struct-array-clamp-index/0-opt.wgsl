struct S {
  a : i32,
  b : i32,
  c : i32,
}

struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var A : array<S, 2u>;
  let x_29 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_31 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_33 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_35 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  A[x_29] = S(x_31, x_33, x_35);
  let x_39 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  let x_41 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_43 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_45 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  A[x_39] = S(x_41, x_43, x_45);
  let x_49 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_51 : i32 = A[x_49].b;
  let x_53 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  if ((x_51 == x_53)) {
    let x_58 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_61 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    A[clamp(x_58, 1, 2)].b = x_61;
  }
  let x_64 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  let x_66 : i32 = A[x_64].b;
  let x_68 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  if ((x_66 == x_68)) {
    let x_74 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_77 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_80 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_83 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_74), f32(x_77), f32(x_80), f32(x_83));
  } else {
    let x_87 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_88 : f32 = f32(x_87);
    x_GLF_color = vec4<f32>(x_88, x_88, x_88, x_88);
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
