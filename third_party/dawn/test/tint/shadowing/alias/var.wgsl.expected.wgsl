alias a = i32;

fn f() {
  {
    var a : a = a();
    var b = a;
  }
  var a : a = a();
  var b = a;
}
