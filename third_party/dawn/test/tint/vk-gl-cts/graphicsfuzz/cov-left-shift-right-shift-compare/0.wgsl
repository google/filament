struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_5 : buf0;

fn main_1() {
  var x_32_phi : i32;
  let x_24 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  let x_25 : f32 = f32(x_24);
  x_GLF_color = vec4<f32>(x_25, x_25, x_25, x_25);
  let x_28 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  let x_30 : i32 = ((x_28 << bitcast<u32>(x_28)) >> bitcast<u32>(1));
  x_32_phi = x_24;
  loop {
    let x_32 : i32 = x_32_phi;
    if ((x_30 < 10)) {
    } else {
      break;
    }
    var x_33 : i32;
    x_33 = (x_32 + 1);
    let x_39 : i32 = x_5.x_GLF_uniform_int_values[2].el;
    if ((x_33 == bitcast<i32>(x_39))) {
      let x_43 : f32 = f32(x_28);
      x_GLF_color = vec4<f32>(x_43, x_25, x_25, x_43);
      break;
    }

    continuing {
      x_32_phi = x_33;
    }
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
