var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var m : mat2x2<f32>;
  m = mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
  let x_26 : mat2x2<f32> = m;
  let x_28 : mat2x2<f32> = m;
  let x_30 : mat2x2<f32> = (transpose(x_26) * transpose(x_28));
  let x_31 : mat2x2<f32> = m;
  let x_32 : mat2x2<f32> = m;
  let x_34 : mat2x2<f32> = transpose((x_31 * x_32));
  if ((all((x_30[0u] == x_34[0u])) & all((x_30[1u] == x_34[1u])))) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
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
