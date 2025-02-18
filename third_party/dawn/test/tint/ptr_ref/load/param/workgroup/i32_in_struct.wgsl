struct str {
  i : i32,
};

var<workgroup> S : str;

fn func(pointer : ptr<workgroup, i32>) -> i32 {
  return *pointer;
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&S.i);
}
