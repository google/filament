struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 3u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var A1 : array<f32, 3u>;
  var a : i32;
  var b : f32;
  var c : bool;
  var x_36 : bool;
  let x_38 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  let x_40 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_42 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  A1 = array<f32, 3u>(x_38, x_40, x_42);
  let x_45 : i32 = x_9.x_GLF_uniform_int_values[0].el;
  let x_47 : i32 = x_9.x_GLF_uniform_int_values[0].el;
  let x_49 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  a = clamp(x_45, x_47, x_49);
  let x_51 : i32 = a;
  let x_53 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  let x_55 : i32 = x_9.x_GLF_uniform_int_values[0].el;
  let x_58 : f32 = A1[clamp(x_51, x_53, x_55)];
  b = x_58;
  let x_59 : f32 = b;
  let x_61 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  let x_63 : f32 = A1[x_61];
  if ((x_59 < x_63)) {
    let x_69 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_71 : f32 = x_6.x_GLF_uniform_float_values[2].el;
    x_36 = (x_69 > x_71);
  } else {
    let x_74 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_76 : i32 = x_9.x_GLF_uniform_int_values[2].el;
    let x_78 : f32 = A1[x_76];
    x_36 = (x_74 < x_78);
  }
  let x_80 : bool = x_36;
  c = x_80;
  let x_81 : bool = c;
  if (x_81) {
    let x_86 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_89 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_92 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_95 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_86), f32(x_89), f32(x_92), f32(x_95));
  } else {
    let x_99 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4<f32>(x_99, x_99, x_99, x_99);
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
