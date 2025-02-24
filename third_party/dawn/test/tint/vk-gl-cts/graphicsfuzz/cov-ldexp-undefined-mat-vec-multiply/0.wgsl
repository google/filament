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
  var v1 : vec2<f32>;
  let x_35 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  v1 = vec2<f32>(x_35, x_35);
  let x_38 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  let x_40 : f32 = v1.y;
  v1[x_38] = ldexp(x_40, -256);
  let x_43 : vec2<f32> = v1;
  if ((((x_43 * mat2x2<f32>(vec2<f32>(x_35, 0.0), vec2<f32>(0.0, x_35)))).x == x_35)) {
    let x_53 : f32 = f32(x_38);
    let x_55 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    let x_56 : f32 = f32(x_55);
    x_GLF_color = vec4<f32>(x_53, x_56, x_56, x_53);
  } else {
    let x_59 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    let x_60 : f32 = f32(x_59);
    x_GLF_color = vec4<f32>(x_60, x_60, x_60, x_60);
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
