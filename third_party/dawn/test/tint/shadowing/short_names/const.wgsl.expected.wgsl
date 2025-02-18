alias a = vec3f;

fn f() {
  {
    const vec3f = 1;
    const b = vec3f;
  }
  const c = a();
  const d = vec3f();
}
