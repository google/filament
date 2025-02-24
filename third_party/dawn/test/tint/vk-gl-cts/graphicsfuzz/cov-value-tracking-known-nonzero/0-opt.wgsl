struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 4u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var sum : i32;
  var i : i32;
  a = 65536;
  let x_29 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  sum = x_29;
  let x_31 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  if ((1 == x_31)) {
    let x_35 : i32 = a;
    a = (x_35 - 1);
  }
  i = 0;
  loop {
    let x_41 : i32 = i;
    let x_42 : i32 = a;
    if ((x_41 < x_42)) {
    } else {
      break;
    }
    let x_45 : i32 = i;
    let x_46 : i32 = sum;
    sum = (x_46 + x_45);

    continuing {
      let x_49 : i32 = x_7.x_GLF_uniform_int_values[2].el;
      let x_50 : i32 = i;
      i = (x_50 + x_49);
    }
  }
  let x_52 : i32 = sum;
  let x_54 : i32 = x_7.x_GLF_uniform_int_values[3].el;
  if ((x_52 == x_54)) {
    let x_60 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_63 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_66 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_69 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_60), f32(x_63), f32(x_66), f32(x_69));
  } else {
    let x_73 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_74 : f32 = f32(x_73);
    x_GLF_color = vec4<f32>(x_74, x_74, x_74, x_74);
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
