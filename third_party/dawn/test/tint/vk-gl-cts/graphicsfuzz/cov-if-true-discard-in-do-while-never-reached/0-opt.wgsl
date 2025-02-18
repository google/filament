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
  a = 1;
  loop {
    let x_29 : i32 = a;
    let x_31 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_29 >= x_31)) {
      break;
    }
    if (true) {
      discard;
    }
    let x_37 : i32 = a;
    a = (x_37 + 1);

    continuing {
      let x_39 : i32 = a;
      break if !(x_39 != 1);
    }
  }
  let x_41 : i32 = a;
  if ((x_41 == 1)) {
    let x_47 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_50 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_53 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(1.0, f32(x_47), f32(x_50), f32(x_53));
  } else {
    let x_57 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_58 : f32 = f32(x_57);
    x_GLF_color = vec4<f32>(x_58, x_58, x_58, x_58);
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
