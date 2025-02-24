struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct buf2 {
  zero : i32,
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

@group(0) @binding(2) var<uniform> x_8 : buf2;

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  var x_48 : bool;
  var x_49_phi : bool;
  let x_33 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  f = pow(-(x_33), sinh(1.0));
  let x_37 : f32 = f;
  let x_39 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_40 : bool = (x_37 == x_39);
  x_49_phi = x_40;
  if (!(x_40)) {
    let x_45 : i32 = x_8.zero;
    let x_47 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    x_48 = (x_45 == x_47);
    x_49_phi = x_48;
  }
  let x_49 : bool = x_49_phi;
  if (x_49) {
    let x_54 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_57 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_60 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_63 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_54), f32(x_57), f32(x_60), f32(x_63));
  } else {
    let x_67 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_68 : f32 = f32(x_67);
    x_GLF_color = vec4<f32>(x_68, x_68, x_68, x_68);
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
