struct buf1 {
  v1 : vec2<f32>,
}

struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(1) var<uniform> x_5 : buf1;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

fn main_1() {
  let x_29 : f32 = x_5.v1.x;
  let x_31 : f32 = x_5.v1.y;
  if ((x_29 < x_31)) {
    let x_37 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_40 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_43 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_46 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_37), f32(x_40), f32(x_43), f32(x_46));
    let x_50 : f32 = x_5.v1.x;
    let x_52 : f32 = x_5.v1.y;
    if ((x_50 > x_52)) {
      let x_57 : i32 = x_7.x_GLF_uniform_int_values[0].el;
      let x_58 : f32 = f32(x_57);
      x_GLF_color = vec4<f32>(x_58, x_58, x_58, x_58);
    }
    return;
  } else {
    let x_61 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_62 : f32 = f32(x_61);
    x_GLF_color = vec4<f32>(x_62, x_62, x_62, x_62);
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
