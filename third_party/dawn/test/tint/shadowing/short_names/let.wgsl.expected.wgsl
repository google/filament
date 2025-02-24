alias a = vec3f;

fn f() {
  {
    let vec3f = 1;
    let b = vec3f;
  }
  let c = a();
  let d = vec3f();
}
