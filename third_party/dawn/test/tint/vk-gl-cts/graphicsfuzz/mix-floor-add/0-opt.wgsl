var<private> x_GLF_color : vec4<f32>;

fn GLF_live6mand_() -> vec3<f32> {
  return mix(bitcast<vec3<f32>>(vec3<u32>(38730u, 63193u, 63173u)), vec3<f32>(463.0, 4.0, 0.0), vec3<f32>(2.0, 2.0, 2.0));
}

fn main_1() {
  let x_27 : vec3<f32> = GLF_live6mand_();
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
