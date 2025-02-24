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

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(1) var<uniform> x_8 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var color : vec4<f32>;
  let x_29 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_31 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_33 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_35 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  color = vec4<f32>(x_29, x_31, x_33, x_35);
  let x_38 : i32 = x_8.x_GLF_uniform_int_values[1].el;
  switch(((1 | x_38) ^ 1)) {
    case 0: {
      let x_44 : i32 = x_8.x_GLF_uniform_int_values[0].el;
      let x_46 : f32 = x_6.x_GLF_uniform_float_values[1].el;
      color[x_44] = x_46;
    }
    default: {
    }
  }
  let x_48 : vec4<f32> = color;
  x_GLF_color = x_48;
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
