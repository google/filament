struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  let x_25 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  a = x_25;
  loop {
    let x_30 : i32 = a;
    if ((x_30 >= 0)) {
    } else {
      break;
    }
    let x_33 : i32 = a;
    a = ((x_33 / 2) - 1);
  }
  let x_36 : i32 = a;
  let x_38 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  if ((x_36 == -(x_38))) {
    let x_45 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_48 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_51 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_54 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_45), f32(x_48), f32(x_51), f32(x_54));
  } else {
    let x_58 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_59 : f32 = f32(x_58);
    x_GLF_color = vec4<f32>(x_59, x_59, x_59, x_59);
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
