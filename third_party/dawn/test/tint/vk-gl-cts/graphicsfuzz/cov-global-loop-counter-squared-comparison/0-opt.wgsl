struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

var<private> x_GLF_global_loop_count : i32;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

fn main_1() {
  x_GLF_global_loop_count = 0;
  loop {
    let x_30 : i32 = x_GLF_global_loop_count;
    if ((x_30 < 100)) {
    } else {
      break;
    }
    let x_33 : i32 = x_GLF_global_loop_count;
    x_GLF_global_loop_count = (x_33 + 1);
    let x_35 : i32 = x_GLF_global_loop_count;
    let x_36 : i32 = x_GLF_global_loop_count;
    if (((x_35 * x_36) > 10)) {
      break;
    }
  }
  let x_41 : i32 = x_GLF_global_loop_count;
  if ((x_41 == 4)) {
    let x_47 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_50 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_53 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_56 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_47), f32(x_50), f32(x_53), f32(x_56));
  } else {
    let x_60 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_61 : f32 = f32(x_60);
    x_GLF_color = vec4<f32>(x_61, x_61, x_61, x_61);
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
