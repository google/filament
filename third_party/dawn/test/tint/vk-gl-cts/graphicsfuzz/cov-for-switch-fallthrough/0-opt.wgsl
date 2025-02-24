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
  var a : i32;
  var i : i32;
  let x_26 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  a = x_26;
  let x_28 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  i = x_28;
  loop {
    let x_33 : i32 = i;
    let x_35 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_33 < x_35)) {
    } else {
      break;
    }
    let x_38 : i32 = i;
    switch(x_38) {
      case 0, -1: {
        let x_42 : i32 = x_6.x_GLF_uniform_int_values[1].el;
        a = x_42;
      }
      default: {
      }
    }

    continuing {
      let x_43 : i32 = i;
      i = (x_43 + 1);
    }
  }
  let x_45 : i32 = a;
  let x_47 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  if ((x_45 == x_47)) {
    let x_53 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_56 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_59 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_62 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_53), f32(x_56), f32(x_59), f32(x_62));
  } else {
    let x_66 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_67 : f32 = f32(x_66);
    x_GLF_color = vec4<f32>(x_67, x_67, x_67, x_67);
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
