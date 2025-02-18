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
  var count : i32;
  var i : i32;
  let x_27 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  count = x_27;
  let x_29 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  i = x_29;
  loop {
    let x_34 : i32 = i;
    let x_36 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_34 < x_36)) {
    } else {
      break;
    }
    let x_39 : i32 = count;
    let x_42 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    if (((x_39 % -93448) > x_42)) {
      let x_46 : i32 = count;
      count = (x_46 + 1);
    }

    continuing {
      let x_48 : i32 = i;
      i = (x_48 + 1);
    }
  }
  let x_50 : i32 = count;
  let x_52 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  if ((x_50 == x_52)) {
    let x_58 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_61 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_64 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_67 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_58), f32(x_61), f32(x_64), f32(x_67));
  } else {
    let x_71 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_72 : f32 = f32(x_71);
    x_GLF_color = vec4<f32>(x_72, x_72, x_72, x_72);
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
