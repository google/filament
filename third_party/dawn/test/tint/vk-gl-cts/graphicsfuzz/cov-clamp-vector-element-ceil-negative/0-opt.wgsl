struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

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
  let x_36 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  v0 = vec2<f32>(x_36, -580.015014648);
  let x_38 : vec2<f32> = v0;
  let x_41 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_42 : f32 = -(x_41);
  v1 = clamp(ceil(x_38), vec2<f32>(x_42, x_42), vec2<f32>(100.0, 100.0));
  let x_46 : f32 = v1.y;
  let x_48 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_46 == -(x_48))) {
    let x_55 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_58 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_61 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_64 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_55), f32(x_58), f32(x_61), f32(x_64));
  } else {
    let x_68 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_69 : f32 = f32(x_68);
    x_GLF_color = vec4<f32>(x_69, x_69, x_69, x_69);
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
