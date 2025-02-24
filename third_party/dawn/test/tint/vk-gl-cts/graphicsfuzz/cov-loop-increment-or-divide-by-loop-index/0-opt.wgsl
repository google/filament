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
  let x_27 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  a = x_27;
  i = 0;
  loop {
    let x_32 : i32 = i;
    if ((x_32 < 3)) {
    } else {
      break;
    }
    let x_35 : i32 = i;
    let x_37 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    if ((x_35 == x_37)) {
      let x_42 : i32 = a;
      a = (x_42 + 1);
    } else {
      let x_44 : i32 = a;
      let x_45 : i32 = i;
      a = (x_44 / x_45);
    }

    continuing {
      let x_47 : i32 = i;
      i = (x_47 + 1);
    }
  }
  let x_49 : i32 = a;
  let x_51 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  if ((x_49 == x_51)) {
    let x_57 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_60 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_63 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_66 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_57), f32(x_60), f32(x_63), f32(x_66));
  } else {
    let x_70 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_71 : f32 = f32(x_70);
    x_GLF_color = vec4<f32>(x_71, x_71, x_71, x_71);
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
