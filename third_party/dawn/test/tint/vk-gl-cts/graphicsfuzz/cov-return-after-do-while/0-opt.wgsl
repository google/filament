struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_5 : buf1;

fn main_1() {
  let x_22 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  let x_25 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  let x_28 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  let x_31 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  x_GLF_color = vec4<f32>(f32(x_22), f32(x_25), f32(x_28), f32(x_31));
  let x_35 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  let x_37 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  if ((x_35 > x_37)) {
    loop {
      let x_46 : i32 = x_5.x_GLF_uniform_int_values[0].el;
      let x_47 : f32 = f32(x_46);
      x_GLF_color = vec4<f32>(x_47, x_47, x_47, x_47);

      continuing {
        let x_50 : i32 = x_5.x_GLF_uniform_int_values[1].el;
        let x_52 : i32 = x_5.x_GLF_uniform_int_values[0].el;
        break if !(x_50 > x_52);
      }
    }
    return;
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
