@group(0) @binding(0) var<storage, read_write> s : i32;

alias A = i32;

alias _A = i32;

alias B = A;

alias _B = _A;

@compute @workgroup_size(1)
fn f() {
  var c : B;
  var d : _B;
  s = (c + d);
}
