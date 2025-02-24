struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

var<private> x_GLF_global_loop_count : i32;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

fn main_1() {
  x_GLF_global_loop_count = 0;
  let x_26 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_29 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_32 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_35 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  x_GLF_color = vec4<f32>(f32(x_26), f32(x_29), f32(x_32), f32(x_35));
  loop {
    var x_54 : bool;
    var x_55_phi : bool;
    let x_42 : i32 = x_GLF_global_loop_count;
    if ((x_42 < 100)) {
    } else {
      break;
    }
    let x_45 : i32 = x_GLF_global_loop_count;
    x_GLF_global_loop_count = (x_45 + 1);
    x_55_phi = true;
    if (!(true)) {
      let x_51 : i32 = x_6.x_GLF_uniform_int_values[0].el;
      let x_53 : i32 = x_6.x_GLF_uniform_int_values[1].el;
      x_54 = (x_51 == x_53);
      x_55_phi = x_54;
    }
    let x_55 : bool = x_55_phi;
    if (!(x_55)) {
      break;
    }
  }
  loop {
    let x_63 : i32 = x_GLF_global_loop_count;
    if ((x_63 < 100)) {
    } else {
      break;
    }
    let x_66 : i32 = x_GLF_global_loop_count;
    x_GLF_global_loop_count = (x_66 + 1);
    let x_69 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_70 : f32 = f32(x_69);
    x_GLF_color = vec4<f32>(x_70, x_70, x_70, x_70);
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
