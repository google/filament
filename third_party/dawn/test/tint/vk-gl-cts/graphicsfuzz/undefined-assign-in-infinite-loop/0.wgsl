struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

fn main_1() {
  var GLF_dead6index : i32;
  var GLF_dead6currentNode : i32;
  var donor_replacementGLF_dead6tree : array<i32, 1u>;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  GLF_dead6index = 0;
  let x_34 : f32 = x_6.injectionSwitch.y;
  if ((x_34 < 0.0)) {
    loop {
      if (true) {
      } else {
        break;
      }
      let x_9 : i32 = GLF_dead6index;
      let x_10 : i32 = donor_replacementGLF_dead6tree[x_9];
      GLF_dead6currentNode = x_10;
      let x_11 : i32 = GLF_dead6currentNode;
      GLF_dead6index = x_11;
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
