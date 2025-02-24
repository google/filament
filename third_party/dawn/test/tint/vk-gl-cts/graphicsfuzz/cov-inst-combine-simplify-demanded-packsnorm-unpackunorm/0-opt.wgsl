struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 4u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 4u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : u32;
  var v1 : vec4<f32>;
  var E : f32;
  var x_69 : bool;
  var x_85 : bool;
  var x_101 : bool;
  var x_70_phi : bool;
  var x_86_phi : bool;
  var x_102_phi : bool;
  let x_35 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  a = pack4x8snorm(vec4<f32>(x_35, x_35, x_35, x_35));
  let x_38 : u32 = a;
  v1 = unpack4x8unorm(x_38);
  let x_41 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  E = x_41;
  let x_43 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  let x_45 : f32 = v1[x_43];
  let x_47 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  let x_49 : f32 = x_6.x_GLF_uniform_float_values[3].el;
  let x_53 : f32 = E;
  let x_54 : bool = (abs((x_45 - (x_47 / x_49))) < x_53);
  x_70_phi = x_54;
  if (x_54) {
    let x_58 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_60 : f32 = v1[x_58];
    let x_62 : f32 = x_6.x_GLF_uniform_float_values[2].el;
    let x_64 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    let x_68 : f32 = E;
    x_69 = (abs((x_60 - (x_62 / x_64))) < x_68);
    x_70_phi = x_69;
  }
  let x_70 : bool = x_70_phi;
  x_86_phi = x_70;
  if (x_70) {
    let x_74 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_76 : f32 = v1[x_74];
    let x_78 : f32 = x_6.x_GLF_uniform_float_values[2].el;
    let x_80 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    let x_84 : f32 = E;
    x_85 = (abs((x_76 - (x_78 / x_80))) < x_84);
    x_86_phi = x_85;
  }
  let x_86 : bool = x_86_phi;
  x_102_phi = x_86;
  if (x_86) {
    let x_90 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_92 : f32 = v1[x_90];
    let x_94 : f32 = x_6.x_GLF_uniform_float_values[2].el;
    let x_96 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    let x_100 : f32 = E;
    x_101 = (abs((x_92 - (x_94 / x_96))) < x_100);
    x_102_phi = x_101;
  }
  let x_102 : bool = x_102_phi;
  if (x_102) {
    let x_107 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_110 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_113 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_116 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_107), f32(x_110), f32(x_113), f32(x_116));
  } else {
    let x_120 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_121 : f32 = f32(x_120);
    x_GLF_color = vec4<f32>(x_121, x_121, x_121, x_121);
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
