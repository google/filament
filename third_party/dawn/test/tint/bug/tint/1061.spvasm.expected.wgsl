struct buf0 {
  /* @offset(0) */
  r : vec4f,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4f;

fn main_1() {
  var f : f32;
  var v : vec4f;
  f = determinant(mat3x3f(vec3f(1.0f, 0.0f, 0.0f), vec3f(0.0f, 1.0f, 0.0f), vec3f(0.0f, 0.0f, 1.0f)));
  v = vec4f(sin(f), cos(f), exp2(f), log(f));
  if ((distance(v, x_7.r) < 0.10000000149011611938f)) {
    x_GLF_color = vec4f(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4f();
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4f,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
