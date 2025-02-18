struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_8 : buf0;

fn main_1() {
  var i : i32;
  let x_32 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  i = x_32;
  loop {
    let x_37 : i32 = i;
    if ((x_37 >= 0)) {
    } else {
      break;
    }
    let x_40 : i32 = i;
    if (((x_40 % 2) == 0)) {
      let x_47 : i32 = x_6.x_GLF_uniform_int_values[0].el;
      let x_50 : i32 = x_6.x_GLF_uniform_int_values[0].el;
      let x_53 : i32 = x_6.x_GLF_uniform_int_values[1].el;
      x_GLF_color = vec4<f32>(1.0, f32(x_47), f32(x_50), f32(x_53));
    } else {
      let x_57 : f32 = x_8.x_GLF_uniform_float_values[0].el;
      x_GLF_color = vec4<f32>(x_57, x_57, x_57, x_57);
    }
    let x_59 : i32 = i;
    i = (x_59 - 1);
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
