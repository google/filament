var<private> color : vec4<f32>;

fn drawShape_vf2_(pos : ptr<function, vec2<f32>>) -> vec3<f32> {
  var c3 : bool;
  var x_35_phi : bool;
  let x_32 : f32 = (*(pos)).y;
  let x_33 : bool = (x_32 < 1.0);
  c3 = x_33;
  x_35_phi = x_33;
  loop {
    let x_35 : bool = x_35_phi;
    if (x_35) {
    } else {
      break;
    }
    return vec3<f32>(1.0, 1.0, 1.0);

    continuing {
      x_35_phi = false;
    }
  }
  return vec3<f32>(1.0, 1.0, 1.0);
}

fn main_1() {
  var param : vec2<f32>;
  param = vec2<f32>(1.0, 1.0);
  let x_29 : vec3<f32> = drawShape_vf2_(&(param));
  color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  return;
}

struct main_out {
  @location(0)
  color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(color);
}
