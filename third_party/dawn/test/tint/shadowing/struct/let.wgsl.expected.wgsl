struct a {
  a : i32,
}

fn f() {
  {
    let a : a = a();
    let b = a;
  }
  let a : a = a();
  let b = a;
}
