struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var i : i32;
  let x_25 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  a = x_25;
  let x_27 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  i = -(x_27);
  loop {
    let x_33 : i32 = i;
    let x_35 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_38 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if (((x_33 | x_35) < x_38)) {
    } else {
      break;
    }
    let x_41 : i32 = i;
    let x_43 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    a = (x_41 * x_43);

    continuing {
      let x_45 : i32 = i;
      i = (x_45 + 1);
    }
  }
  let x_47 : i32 = a;
  let x_49 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  if ((x_47 == -(x_49))) {
    let x_56 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_59 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_62 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_65 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_56), f32(x_59), f32(x_62), f32(x_65));
  } else {
    let x_68 : i32 = a;
    let x_69 : f32 = f32(x_68);
    x_GLF_color = vec4<f32>(x_69, x_69, x_69, x_69);
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
