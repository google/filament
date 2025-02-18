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
  var m : mat2x2<f32>;
  let x_29 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_30 : f32 = f32(x_29);
  m = transpose(transpose(mat2x2<f32>(vec2<f32>(x_30, 0.0), vec2<f32>(0.0, x_30))));
  let x_36 : mat2x2<f32> = m;
  let x_38 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_39 : f32 = f32(x_38);
  let x_42 : mat2x2<f32> = mat2x2<f32>(vec2<f32>(x_39, 0.0), vec2<f32>(0.0, x_39));
  if ((all((x_36[0u] == x_42[0u])) & all((x_36[1u] == x_42[1u])))) {
    let x_56 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_59 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_62 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_65 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_56), f32(x_59), f32(x_62), f32(x_65));
  } else {
    let x_69 : i32 = x_6.x_GLF_uniform_int_values[1].el;
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
