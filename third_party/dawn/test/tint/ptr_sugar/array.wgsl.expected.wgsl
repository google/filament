fn deref_const() {
  var a : array<i32, 10>;
  let p = &(a);
  var b = (*(p))[0];
  (*(p))[0] = 42;
}

fn no_deref_const() {
  var a : array<i32, 10>;
  let p = &(a);
  var b = p[0];
  p[0] = 42;
}

fn deref_let() {
  var a : array<i32, 10>;
  let p = &(a);
  let i = 0;
  var b = (*(p))[i];
  (*(p))[0] = 42;
}

fn no_deref_let() {
  var a : array<i32, 10>;
  let p = &(a);
  let i = 0;
  var b = p[i];
  p[0] = 42;
}

fn deref_var() {
  var a : array<i32, 10>;
  let p = &(a);
  var i = 0;
  var b = (*(p))[i];
  (*(p))[0] = 42;
}

fn no_deref_var() {
  var a : array<i32, 10>;
  let p = &(a);
  var i = 0;
  var b = p[i];
  p[0] = 42;
}

@compute @workgroup_size(1)
fn main() {
  deref_const();
  no_deref_const();
  deref_let();
  no_deref_let();
  deref_var();
  no_deref_var();
}
