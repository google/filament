struct S {
  a : array<vec4<i32>, 4>
}

@group(0) @binding(0) var<storage, read_write>  buffer : array<S>;

var<private> v : u32;

fn idx1() -> i32 {
  v++;
  return 1;
}

fn idx2() -> i32 {
  v++;
  return 2;
}

fn idx3() -> i32 {
  v++;
  return 3;
}

fn idx4() -> i32 {
  v++;
  return 4;
}

fn idx5() -> i32 {
  v++;
  return 0;
}

fn idx6() -> i32 {
  v++;
  return 2;
}

fn main() {
  for (buffer[idx1()].a[idx2()][idx3()]++;
       v < 10u;
       buffer[idx4()].a[idx5()][idx6()]++) {
  }
}
