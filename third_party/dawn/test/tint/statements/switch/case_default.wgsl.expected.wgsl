@compute @workgroup_size(1)
fn f() {
  var i : i32;
  var result : i32;
  switch(i) {
    default: {
      result = 10;
    }
    case 1: {
      result = 22;
    }
    case 2: {
      result = 33;
    }
  }
}
