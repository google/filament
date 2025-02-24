struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 4u>;

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

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(1) var<uniform> x_10 : buf1;

fn main_1() {
  var a : i32;
  var b : f32;
  let x_39 : f32 = gl_FragCoord.y;
  let x_41 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  a = select(2, 0, (x_39 >= x_41));
  let x_45 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  let x_47 : f32 = x_7.x_GLF_uniform_float_values[2].el;
  let x_49 : f32 = x_7.x_GLF_uniform_float_values[3].el;
  let x_51 : i32 = a;
  b = vec3<f32>(x_45, x_47, x_49)[x_51];
  let x_53 : f32 = b;
  let x_55 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  if ((x_53 == x_55)) {
    let x_61 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_64 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_67 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_70 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_61), f32(x_64), f32(x_67), f32(x_70));
  } else {
    let x_74 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_75 : f32 = f32(x_74);
    x_GLF_color = vec4<f32>(x_75, x_75, x_75, x_75);
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
