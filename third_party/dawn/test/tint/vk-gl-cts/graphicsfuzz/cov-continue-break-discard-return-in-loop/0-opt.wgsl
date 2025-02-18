struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 4u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

struct buf1 {
  zero : i32,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_5 : buf0;

@group(0) @binding(1) var<uniform> x_7 : buf1;

fn main_1() {
  let x_28 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  let x_31 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  let x_34 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  let x_37 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  x_GLF_color = vec4<f32>(f32(x_28), f32(x_31), f32(x_34), f32(x_37));
  loop {
    let x_45 : i32 = x_7.zero;
    let x_47 : i32 = x_5.x_GLF_uniform_int_values[0].el;
    if ((x_45 == x_47)) {
      continue;
    }
    let x_52 : i32 = x_7.zero;
    let x_54 : i32 = x_5.x_GLF_uniform_int_values[2].el;
    if ((x_52 == x_54)) {
      break;
    }
    let x_59 : i32 = x_7.zero;
    let x_61 : i32 = x_5.x_GLF_uniform_int_values[3].el;
    if ((x_59 == x_61)) {
      discard;
    }
    return;

    continuing {
      break if !(false);
    }
  }
  let x_66 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  let x_67 : f32 = f32(x_66);
  x_GLF_color = vec4<f32>(x_67, x_67, x_67, x_67);
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
