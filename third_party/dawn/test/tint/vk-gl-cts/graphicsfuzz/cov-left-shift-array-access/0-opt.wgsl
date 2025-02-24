struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var idx : i32;
  var a : i32;
  var indexable : Arr;
  let x_27 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  idx = (1 << bitcast<u32>(x_27));
  let x_30 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_32 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_34 : i32 = idx;
  indexable = Arr(strided_arr(x_30), strided_arr(x_32));
  let x_36 : i32 = indexable[x_34].el;
  a = x_36;
  let x_37 : i32 = a;
  let x_39 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  if ((x_37 == x_39)) {
    let x_45 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_48 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_51 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_54 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_45), f32(x_48), f32(x_51), f32(x_54));
  } else {
    let x_58 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_59 : f32 = f32(x_58);
    x_GLF_color = vec4<f32>(x_59, x_59, x_59, x_59);
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
