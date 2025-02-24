struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 4u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v1 : vec2<f32>;
  var v2 : vec2<i32>;
  var v3 : vec2<f32>;
  var x_66 : bool;
  var x_67_phi : bool;
  let x_41 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  let x_43 : f32 = x_6.x_GLF_uniform_float_values[3].el;
  v1 = sinh(vec2<f32>(x_41, x_43));
  let x_47 : i32 = x_9.x_GLF_uniform_int_values[0].el;
  v2 = vec2<i32>(x_47, -3000);
  let x_49 : vec2<f32> = v1;
  let x_50 : vec2<i32> = v2;
  v3 = ldexp(x_49, x_50);
  let x_53 : f32 = v3.y;
  x_GLF_color = vec4<f32>(x_53, x_53, x_53, x_53);
  let x_56 : f32 = v3.x;
  let x_58 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_59 : bool = (x_56 > x_58);
  x_67_phi = x_59;
  if (x_59) {
    let x_63 : f32 = v3.x;
    let x_65 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    x_66 = (x_63 < x_65);
    x_67_phi = x_66;
  }
  let x_67 : bool = x_67_phi;
  if (x_67) {
    let x_72 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_75 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_78 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_81 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_72), f32(x_75), f32(x_78), f32(x_81));
  } else {
    let x_85 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_86 : f32 = f32(x_85);
    x_GLF_color = vec4<f32>(x_86, x_86, x_86, x_86);
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
