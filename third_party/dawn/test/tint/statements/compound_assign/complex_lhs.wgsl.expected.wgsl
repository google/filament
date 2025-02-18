struct S {
  a : array<vec4<i32>, 4>,
}

var<private> counter : i32;

fn foo() -> i32 {
  counter += 1;
  return counter;
}

fn bar() -> i32 {
  counter += 2;
  return counter;
}

fn main() {
  var x = S();
  let p = &(x);
  (*(p)).a[foo()][bar()] += 5;
}
