struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> GLF_live6tree : array<i32, 10u>;

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn GLF_live6search_() -> i32 {
  var GLF_live6index : i32;
  var GLF_live6currentNode : i32;
  GLF_live6index = 0;
  loop {
    if (true) {
    } else {
      break;
    }
    let x_10 : i32 = GLF_live6index;
    let x_11 : i32 = GLF_live6tree[x_10];
    GLF_live6currentNode = x_11;
    let x_12 : i32 = GLF_live6currentNode;
    if ((x_12 != 1)) {
      return 1;
    }
    GLF_live6index = 1;
  }
  return 1;
}

fn main_1() {
  let x_40 : f32 = x_9.injectionSwitch.x;
  if ((x_40 > 1.0)) {
    let x_13 : i32 = GLF_live6search_();
  }
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
