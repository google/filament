struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 3u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

var<private> x_GLF_global_loop_count : i32;

@group(0) @binding(0) var<uniform> x_7 : buf0;

@group(0) @binding(1) var<uniform> x_10 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  var i : i32;
  x_GLF_global_loop_count = 0;
  let x_36 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  f = x_36;
  let x_38 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  i = x_38;
  loop {
    let x_43 : i32 = i;
    let x_45 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    if ((x_43 < x_45)) {
    } else {
      break;
    }
    let x_48 : f32 = f;
    let x_50 : f32 = x_7.x_GLF_uniform_float_values[1].el;
    if ((x_48 > x_50)) {
    }
    f = 1.0;
    let x_55 : f32 = x_7.x_GLF_uniform_float_values[2].el;
    let x_56 : f32 = f;
    let x_59 : i32 = i;
    f = ((1.0 - clamp(x_55, 1.0, x_56)) + f32(x_59));

    continuing {
      let x_62 : i32 = i;
      i = (x_62 + 1);
    }
  }
  let x_64 : f32 = f;
  let x_66 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  if ((x_64 == x_66)) {
    let x_72 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_75 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_78 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_81 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_72), f32(x_75), f32(x_78), f32(x_81));
  } else {
    let x_85 : i32 = x_10.x_GLF_uniform_int_values[1].el;
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
