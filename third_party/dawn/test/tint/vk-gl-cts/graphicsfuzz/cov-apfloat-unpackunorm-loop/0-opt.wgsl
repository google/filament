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
  var i : i32;
  var v : vec4<f32>;
  let x_30 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  i = x_30;
  loop {
    let x_35 : i32 = i;
    let x_37 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    if ((x_35 < x_37)) {
    } else {
      break;
    }
    v = unpack4x8unorm(100u);
    let x_42 : f32 = v.x;
    let x_44 : i32 = i;
    if ((i32(x_42) > x_44)) {
      let x_49 : i32 = x_6.x_GLF_uniform_int_values[1].el;
      let x_50 : f32 = f32(x_49);
      x_GLF_color = vec4<f32>(x_50, x_50, x_50, x_50);
      return;
    }

    continuing {
      let x_52 : i32 = i;
      i = (x_52 + 1);
    }
  }
  let x_55 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_58 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_61 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_64 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  x_GLF_color = vec4<f32>(f32(x_55), f32(x_58), f32(x_61), f32(x_64));
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
