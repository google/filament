struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

var<private> x_GLF_global_loop_count : i32;

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_() -> i32 {
  loop {
    let x_72 : i32 = x_GLF_global_loop_count;
    if ((x_72 < 100)) {
    } else {
      break;
    }
    let x_75 : i32 = x_GLF_global_loop_count;
    x_GLF_global_loop_count = (x_75 + 1);
    let x_78 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    return x_78;
  }
  let x_80 : i32 = x_7.x_GLF_uniform_int_values[2].el;
  return x_80;
}

fn main_1() {
  var a : i32;
  x_GLF_global_loop_count = 0;
  loop {
    let x_35 : i32 = x_GLF_global_loop_count;
    x_GLF_global_loop_count = (x_35 + 1);
    if (false) {
      return;
    }

    continuing {
      let x_39 : i32 = x_GLF_global_loop_count;
      break if !(true & (x_39 < 100));
    }
  }
  let x_42 : i32 = func_();
  a = x_42;
  let x_43 : i32 = a;
  let x_45 : i32 = x_7.x_GLF_uniform_int_values[2].el;
  if ((x_43 == x_45)) {
    let x_51 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_54 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_57 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_60 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_51), f32(x_54), f32(x_57), f32(x_60));
  } else {
    let x_64 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_65 : f32 = f32(x_64);
    x_GLF_color = vec4<f32>(x_65, x_65, x_65, x_65);
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
