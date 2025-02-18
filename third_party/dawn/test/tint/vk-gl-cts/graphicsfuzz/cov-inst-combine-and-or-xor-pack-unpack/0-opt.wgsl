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

alias Arr_1 = array<strided_arr_1, 4u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(1) var<uniform> x_10 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : u32;
  var v1 : vec4<f32>;
  var r : vec4<f32>;
  var x_85 : bool;
  var x_97 : bool;
  var x_109 : bool;
  var x_86_phi : bool;
  var x_98_phi : bool;
  var x_110_phi : bool;
  let x_36 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  a = pack4x8unorm(vec4<f32>(x_36, x_36, x_36, x_36));
  let x_39 : u32 = a;
  v1 = unpack4x8snorm(x_39);
  let x_42 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_45 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  let x_48 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_51 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  let x_54 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_57 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  let x_60 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_63 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  r = vec4<f32>((-(x_42) / x_45), (-(x_48) / x_51), (-(x_54) / x_57), (-(x_60) / x_63));
  let x_67 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  let x_69 : f32 = v1[x_67];
  let x_71 : i32 = x_10.x_GLF_uniform_int_values[0].el;
  let x_73 : f32 = r[x_71];
  let x_74 : bool = (x_69 == x_73);
  x_86_phi = x_74;
  if (x_74) {
    let x_78 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_80 : f32 = v1[x_78];
    let x_82 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_84 : f32 = r[x_82];
    x_85 = (x_80 == x_84);
    x_86_phi = x_85;
  }
  let x_86 : bool = x_86_phi;
  x_98_phi = x_86;
  if (x_86) {
    let x_90 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_92 : f32 = v1[x_90];
    let x_94 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_96 : f32 = r[x_94];
    x_97 = (x_92 == x_96);
    x_98_phi = x_97;
  }
  let x_98 : bool = x_98_phi;
  x_110_phi = x_98;
  if (x_98) {
    let x_102 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_104 : f32 = v1[x_102];
    let x_106 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_108 : f32 = r[x_106];
    x_109 = (x_104 == x_108);
    x_110_phi = x_109;
  }
  let x_110 : bool = x_110_phi;
  if (x_110) {
    let x_115 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_118 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_121 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_124 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    x_GLF_color = vec4<f32>(f32(x_115), f32(x_118), f32(x_121), f32(x_124));
  } else {
    let x_128 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_130 : f32 = v1[x_128];
    x_GLF_color = vec4<f32>(x_130, x_130, x_130, x_130);
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
