alias RTArr = array<f32>;

struct doesNotMatter {
  x_compute_data : RTArr,
}

@group(0) @binding(0) var<storage, read_write> x_9 : doesNotMatter;

fn nb_mod_() -> f32 {
  var s : f32;
  var i : i32;
  var GLF_live1i : i32;
  var GLF_live1_looplimiter2 : i32;
  var x_51 : f32;
  var x_56_phi : f32;
  s = 0.0;
  i = 5;
  loop {
    var x_50 : f32;
    var x_51_phi : f32;
    x_56_phi = 0.0;
    if ((5 < 800)) {
    } else {
      break;
    }
    GLF_live1i = 0;
    loop {
      x_51_phi = 0.0;
      if ((0 < 20)) {
      } else {
        break;
      }
      if ((0 >= 5)) {
        x_50 = (0.0 + 1.0);
        s = x_50;
        x_51_phi = x_50;
        break;
      }
      return 42.0;
    }
    x_51 = x_51_phi;
    if ((f32(5) <= x_51)) {
      x_56_phi = x_51;
      break;
    }
    return 42.0;
  }
  let x_56 : f32 = x_56_phi;
  return x_56;
}

fn main_1() {
  let x_32 : f32 = nb_mod_();
  x_9.x_compute_data[0] = x_32;
  return;
}

@compute @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
