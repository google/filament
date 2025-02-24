struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 7u>;

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
  var x_75 : bool;
  var x_92 : bool;
  var x_109 : bool;
  var x_76_phi : bool;
  var x_93_phi : bool;
  var x_110_phi : bool;
  let x_41 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_43 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  a = pack2x16unorm(vec2<f32>(x_41, x_43));
  let x_46 : u32 = a;
  v1 = unpack4x8snorm(x_46);
  E = 0.01;
  let x_49 : i32 = x_10.x_GLF_uniform_int_values[2].el;
  let x_51 : f32 = v1[x_49];
  let x_53 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  let x_55 : f32 = x_6.x_GLF_uniform_float_values[3].el;
  let x_59 : f32 = E;
  let x_60 : bool = (abs((x_51 - (x_53 / x_55))) < x_59);
  x_76_phi = x_60;
  if (x_60) {
    let x_64 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_66 : f32 = v1[x_64];
    let x_68 : f32 = x_6.x_GLF_uniform_float_values[4].el;
    let x_70 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    let x_74 : f32 = E;
    x_75 = (abs((x_66 - (x_68 / x_70))) < x_74);
    x_76_phi = x_75;
  }
  let x_76 : bool = x_76_phi;
  x_93_phi = x_76;
  if (x_76) {
    let x_80 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    let x_82 : f32 = v1[x_80];
    let x_84 : f32 = x_6.x_GLF_uniform_float_values[5].el;
    let x_87 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    let x_91 : f32 = E;
    x_92 = (abs((x_82 - (-(x_84) / x_87))) < x_91);
    x_93_phi = x_92;
  }
  let x_93 : bool = x_93_phi;
  x_110_phi = x_93;
  if (x_93) {
    let x_97 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_99 : f32 = v1[x_97];
    let x_101 : f32 = x_6.x_GLF_uniform_float_values[6].el;
    let x_104 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    let x_108 : f32 = E;
    x_109 = (abs((x_99 - (-(x_101) / x_104))) < x_108);
    x_110_phi = x_109;
  }
  let x_110 : bool = x_110_phi;
  if (x_110) {
    let x_115 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_118 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_121 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_124 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_115), f32(x_118), f32(x_121), f32(x_124));
  } else {
    let x_128 : f32 = x_6.x_GLF_uniform_float_values[5].el;
    x_GLF_color = vec4<f32>(x_128, x_128, x_128, x_128);
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
