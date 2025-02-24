struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 4u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 4u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

@group(0) @binding(1) var<uniform> x_10 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : u32;
  var values : vec4<f32>;
  var r : vec4<f32>;
  var x_85 : bool;
  var x_101 : bool;
  var x_117 : bool;
  var x_86_phi : bool;
  var x_102_phi : bool;
  var x_118_phi : bool;
  a = pack2x16float(vec2<f32>(1.0, 1.0));
  let x_38 : u32 = a;
  values = unpack4x8unorm(x_38);
  let x_41 : f32 = x_8.x_GLF_uniform_float_values[3].el;
  let x_43 : f32 = x_8.x_GLF_uniform_float_values[1].el;
  let x_45 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  let x_48 : f32 = x_8.x_GLF_uniform_float_values[3].el;
  let x_50 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  let x_53 : f32 = x_8.x_GLF_uniform_float_values[1].el;
  let x_55 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  r = vec4<f32>(x_41, (x_43 / x_45), (x_48 / x_50), (x_53 / x_55));
  let x_59 : i32 = x_10.x_GLF_uniform_int_values[0].el;
  let x_61 : f32 = values[x_59];
  let x_63 : i32 = x_10.x_GLF_uniform_int_values[0].el;
  let x_65 : f32 = r[x_63];
  let x_69 : f32 = x_8.x_GLF_uniform_float_values[2].el;
  let x_70 : bool = (abs((x_61 - x_65)) < x_69);
  x_86_phi = x_70;
  if (x_70) {
    let x_74 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_76 : f32 = values[x_74];
    let x_78 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_80 : f32 = r[x_78];
    let x_84 : f32 = x_8.x_GLF_uniform_float_values[2].el;
    x_85 = (abs((x_76 - x_80)) < x_84);
    x_86_phi = x_85;
  }
  let x_86 : bool = x_86_phi;
  x_102_phi = x_86;
  if (x_86) {
    let x_90 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_92 : f32 = values[x_90];
    let x_94 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_96 : f32 = r[x_94];
    let x_100 : f32 = x_8.x_GLF_uniform_float_values[2].el;
    x_101 = (abs((x_92 - x_96)) < x_100);
    x_102_phi = x_101;
  }
  let x_102 : bool = x_102_phi;
  x_118_phi = x_102;
  if (x_102) {
    let x_106 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_108 : f32 = values[x_106];
    let x_110 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_112 : f32 = r[x_110];
    let x_116 : f32 = x_8.x_GLF_uniform_float_values[2].el;
    x_117 = (abs((x_108 - x_112)) < x_116);
    x_118_phi = x_117;
  }
  let x_118 : bool = x_118_phi;
  if (x_118) {
    let x_123 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_126 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_129 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_132 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_123), f32(x_126), f32(x_129), f32(x_132));
  } else {
    let x_136 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_137 : f32 = f32(x_136);
    x_GLF_color = vec4<f32>(x_137, x_137, x_137, x_137);
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
