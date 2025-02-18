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
  var arr : array<f32, 3u>;
  var a : i32;
  var x_69 : bool;
  var x_79 : bool;
  var x_70_phi : bool;
  var x_80_phi : bool;
  let x_34 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  let x_36 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_38 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  arr = array<f32, 3u>(x_34, x_36, x_38);
  a = 0;
  loop {
    let x_44 : i32 = a;
    let x_46 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    if ((x_44 <= x_46)) {
    } else {
      break;
    }
    let x_49 : i32 = a;
    a = (x_49 + 1);
    let x_52 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    arr[x_49] = x_52;
  }
  let x_55 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  let x_57 : f32 = arr[x_55];
  let x_59 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_60 : bool = (x_57 == x_59);
  x_70_phi = x_60;
  if (x_60) {
    let x_64 : i32 = x_9.x_GLF_uniform_int_values[2].el;
    let x_66 : f32 = arr[x_64];
    let x_68 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    x_69 = (x_66 == x_68);
    x_70_phi = x_69;
  }
  let x_70 : bool = x_70_phi;
  x_80_phi = x_70;
  if (x_70) {
    let x_74 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_76 : f32 = arr[x_74];
    let x_78 : f32 = x_6.x_GLF_uniform_float_values[2].el;
    x_79 = (x_76 == x_78);
    x_80_phi = x_79;
  }
  let x_80 : bool = x_80_phi;
  if (x_80) {
    let x_85 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_87 : f32 = arr[x_85];
    let x_89 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    let x_91 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    let x_93 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4<f32>(x_87, x_89, x_91, x_93);
  } else {
    let x_96 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    x_GLF_color = vec4<f32>(x_96, x_96, x_96, x_96);
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
