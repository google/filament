struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_() -> i32 {
  var i : i32;
  let x_53 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  i = x_53;
  loop {
    let x_58 : i32 = i;
    i = (x_58 + 1);
    if (true) {
      if (true) {
        let x_65 : i32 = x_7.x_GLF_uniform_int_values[2].el;
        return x_65;
      }
    }

    continuing {
      let x_66 : i32 = i;
      let x_68 : i32 = x_7.x_GLF_uniform_int_values[1].el;
      break if !(x_66 < x_68);
    }
  }
  let x_71 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  return x_71;
}

fn main_1() {
  let x_27 : i32 = func_();
  let x_29 : i32 = x_7.x_GLF_uniform_int_values[2].el;
  if ((x_27 == x_29)) {
    let x_35 : i32 = x_7.x_GLF_uniform_int_values[2].el;
    let x_38 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_41 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_44 : i32 = x_7.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_35), f32(x_38), f32(x_41), f32(x_44));
  } else {
    let x_48 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_49 : f32 = f32(x_48);
    x_GLF_color = vec4<f32>(x_49, x_49, x_49, x_49);
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
