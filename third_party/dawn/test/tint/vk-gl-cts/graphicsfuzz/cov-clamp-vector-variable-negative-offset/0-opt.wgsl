struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 3u>;

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

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_9 : buf0;

fn main_1() {
  var v0 : vec2<f32>;
  var v1 : vec2<f32>;
  let x_37 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  v0 = vec2<f32>(x_37, 3.799999952);
  let x_39 : vec2<f32> = v0;
  let x_43 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  v1 = clamp((x_39 - vec2<f32>(1.0, 1.0)), vec2<f32>(0.0, 0.0), vec2<f32>(x_43, x_43));
  let x_47 : vec2<f32> = v1;
  let x_49 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_51 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  if (all((x_47 == vec2<f32>(x_49, x_51)))) {
    let x_59 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_62 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_65 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_68 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_59), f32(x_62), f32(x_65), f32(x_68));
  } else {
    let x_72 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_73 : f32 = f32(x_72);
    x_GLF_color = vec4<f32>(x_73, x_73, x_73, x_73);
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
