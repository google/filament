struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var c : i32;
  var i : i32;
  let x_27 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  c = x_27;
  let x_29 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  i = x_29;
  loop {
    let x_34 : i32 = i;
    let x_36 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_34 < x_36)) {
    } else {
      break;
    }
    let x_39 : i32 = i;
    c = ~(x_39);
    let x_41 : i32 = c;
    c = clamp(x_41, 0, 3);

    continuing {
      let x_43 : i32 = i;
      i = (x_43 + 1);
    }
  }
  let x_46 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_47 : f32 = f32(x_46);
  x_GLF_color = vec4<f32>(x_47, x_47, x_47, x_47);
  let x_49 : i32 = c;
  let x_51 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  if ((x_49 == x_51)) {
    let x_56 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_59 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_62 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_65 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_56), f32(x_59), f32(x_62), f32(x_65));
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
