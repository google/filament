var<private> x_GLF_color : vec4<f32>;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  var x_37 : mat4x3<f32>;
  var x_38_phi : mat4x3<f32>;
  var x_48_phi : vec3<f32>;
  let x_32 : f32 = gl_FragCoord.y;
  if ((x_32 < 1.0)) {
    x_38_phi = mat4x3<f32>(vec3<f32>(1.0, 0.0, 0.0), vec3<f32>(0.0, 1.0, 0.0), vec3<f32>(0.0, 0.0, 1.0), vec3<f32>(0.0, 0.0, 0.0));
  } else {
    x_37 = transpose(mat3x4<f32>(vec4<f32>(1.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 1.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 1.0, 0.0)));
    x_38_phi = x_37;
  }
  let x_38 : mat4x3<f32> = x_38_phi;
  let x_40 : f32 = transpose(x_38)[0u].y;
  loop {
    if ((x_40 > 1.0)) {
      x_48_phi = vec3<f32>();
      break;
    }
    x_48_phi = vec3<f32>(1.0, 0.0, 0.0);
    break;
  }
  let x_48 : vec3<f32> = x_48_phi;
  x_GLF_color = vec4<f32>(x_48.x, x_48.y, x_48.z, 1.0);
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
