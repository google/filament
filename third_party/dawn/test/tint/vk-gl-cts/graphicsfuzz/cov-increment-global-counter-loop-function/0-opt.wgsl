struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 4u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

var<private> x_GLF_global_loop_count : i32;

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_() {
  var x_66_phi : i32;
  let x_62 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_64 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  x_66_phi = x_64;
  loop {
    var x_67 : i32;
    let x_66 : i32 = x_66_phi;
    let x_70 : i32 = x_7.x_GLF_uniform_int_values[3].el;
    if ((x_66 < x_70)) {
    } else {
      break;
    }

    continuing {
      let x_73 : i32 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_73 + 1);
      x_67 = (x_66 + 1);
      x_66_phi = x_67;
    }
  }
  if ((x_62 < x_62)) {
    return;
  }
  return;
}

fn main_1() {
  x_GLF_global_loop_count = 0;
  loop {
    let x_28 : i32 = x_GLF_global_loop_count;
    if ((x_28 < 10)) {
    } else {
      break;
    }

    continuing {
      let x_32 : i32 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_32 + 1);
      func_();
    }
  }
  loop {
    let x_36 : i32 = x_GLF_global_loop_count;
    if ((x_36 < 10)) {
    } else {
      break;
    }

    continuing {
      let x_40 : i32 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_40 + 1);
    }
  }
  let x_42 : i32 = x_GLF_global_loop_count;
  let x_44 : i32 = x_7.x_GLF_uniform_int_values[2].el;
  if ((x_42 == x_44)) {
    let x_50 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_51 : f32 = f32(x_50);
    let x_53 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_54 : f32 = f32(x_53);
    x_GLF_color = vec4<f32>(x_51, x_54, x_54, x_51);
  } else {
    let x_57 : i32 = x_7.x_GLF_uniform_int_values[0].el;
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
