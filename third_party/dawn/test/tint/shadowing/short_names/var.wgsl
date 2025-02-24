alias a = vec3f;

fn f() {
  {
    var vec3f = 1;
    var b = vec3f;
  }
  var c = a();
  var d = vec3f();
}
