struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 4u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var i : i32;
  let x_27 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  a = x_27;
  let x_29 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  i = -(x_29);
  loop {
    let x_35 : i32 = i;
    let x_36 : i32 = (x_35 + 1);
    i = x_36;
    let x_39 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    if ((reverseBits(x_36) <= x_39)) {
    } else {
      break;
    }
    let x_42 : i32 = a;
    a = (x_42 + 1);
  }
  let x_44 : i32 = a;
  let x_46 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  if ((x_44 == x_46)) {
    let x_52 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_55 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_58 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_61 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_52), f32(x_55), f32(x_58), f32(x_61));
  } else {
    let x_65 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_66 : f32 = f32(x_65);
    x_GLF_color = vec4<f32>(x_66, x_66, x_66, x_66);
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
