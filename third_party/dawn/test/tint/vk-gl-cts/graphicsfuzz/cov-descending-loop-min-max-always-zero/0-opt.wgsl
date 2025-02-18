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

alias Arr_1 = array<strided_arr_1, 3u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(1) var<uniform> x_9 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  var i : i32;
  var a : f32;
  let x_37 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  f = x_37;
  let x_39 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  i = x_39;
  loop {
    let x_44 : i32 = i;
    let x_46 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    if ((x_44 > x_46)) {
    } else {
      break;
    }
    let x_49 : i32 = i;
    a = (1.0 - max(1.0, f32(x_49)));
    let x_53 : f32 = a;
    f = min(max(x_53, 0.0), 0.0);

    continuing {
      let x_56 : i32 = i;
      i = (x_56 - 1);
    }
  }
  let x_58 : f32 = f;
  let x_60 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_58 == x_60)) {
    let x_66 : i32 = x_9.x_GLF_uniform_int_values[2].el;
    let x_68 : f32 = f;
    let x_70 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_66), x_68, f32(x_70), 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
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
