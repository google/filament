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

@group(0) @binding(1) var<uniform> x_11 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : f32;
  var b : f32;
  var c : f32;
  var i : i32;
  let x_35 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  a = x_35;
  let x_37 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  b = x_37;
  let x_39 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  c = x_39;
  let x_41 : i32 = x_11.x_GLF_uniform_int_values[1].el;
  i = x_41;
  loop {
    let x_46 : i32 = i;
    let x_48 : i32 = x_11.x_GLF_uniform_int_values[0].el;
    if ((x_46 < x_48)) {
    } else {
      break;
    }
    let x_51 : i32 = i;
    let x_53 : i32 = x_11.x_GLF_uniform_int_values[2].el;
    if ((x_51 == x_53)) {
      let x_57 : f32 = a;
      let x_60 : f32 = x_6.x_GLF_uniform_float_values[1].el;
      b = (dpdx(x_57) + x_60);
    }
    let x_62 : f32 = a;
    c = dpdx(x_62);
    let x_64 : f32 = c;
    let x_65 : f32 = b;
    a = (x_64 / x_65);

    continuing {
      let x_67 : i32 = i;
      i = (x_67 + 1);
    }
  }
  let x_69 : f32 = a;
  let x_71 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_69 == x_71)) {
    let x_77 : i32 = x_11.x_GLF_uniform_int_values[2].el;
    let x_80 : i32 = x_11.x_GLF_uniform_int_values[1].el;
    let x_83 : i32 = x_11.x_GLF_uniform_int_values[1].el;
    let x_86 : i32 = x_11.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_77), f32(x_80), f32(x_83), f32(x_86));
  } else {
    let x_90 : i32 = x_11.x_GLF_uniform_int_values[1].el;
    let x_91 : f32 = f32(x_90);
    x_GLF_color = vec4<f32>(x_91, x_91, x_91, x_91);
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
