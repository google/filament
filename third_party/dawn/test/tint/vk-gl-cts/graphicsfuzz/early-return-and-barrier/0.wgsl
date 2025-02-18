struct theSSBO {
  out_data : i32,
}

@group(0) @binding(0) var<storage, read_write> x_4 : theSSBO;

fn main_1() {
  var x_30 : mat2x2<f32>;
  var x_30_phi : mat2x2<f32>;
  var x_32_phi : i32;
  x_4.out_data = 42;
  x_30_phi = mat2x2<f32>();
  x_32_phi = 1;
  loop {
    var x_33 : i32;
    x_30 = x_30_phi;
    let x_32 : i32 = x_32_phi;
    if ((x_32 > 0)) {
    } else {
      break;
    }

    continuing {
      x_33 = (x_32 - 1);
      x_30_phi = mat2x2<f32>(vec2<f32>(1.0, 0.0), vec2<f32>(0.0, 1.0));
      x_32_phi = x_33;
    }
  }
  let x_41 : vec2<f32> = (vec2<f32>(1.0, 1.0) * mat2x2<f32>((x_30[0u] - vec2<f32>(1.0, 0.0)), (x_30[1u] - vec2<f32>(0.0, 1.0))));
  loop {
    if ((1.0 < x_41.x)) {
      break;
    }
    workgroupBarrier();
    break;
  }
  return;
}

@compute @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
