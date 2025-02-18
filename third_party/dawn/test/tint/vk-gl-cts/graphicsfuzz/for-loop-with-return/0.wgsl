var<private> x_GLF_color : vec4<f32>;

fn mand_() -> vec3<f32> {
  loop {
    return vec3<f32>(1.0, 1.0, 1.0);
  }
}

fn main_1() {
  let x_17 : vec3<f32> = mand_();
  loop {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    return;
  }
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
