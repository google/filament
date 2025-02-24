fn deref_modf() {
  var a = modf(1.5);
  let p = &a;
  var fract = (*p).fract;
  var whole = (*p).whole;
}

fn no_deref_modf() {
  var a = modf(1.5);
  let p = &a;
  var fract = p.fract;
  var whole = p.whole;
}

fn deref_frexp() {
  var a = frexp(1.5);
  let p = &a;
  var fract = (*p).fract;
  var exp = (*p).exp;
}

fn no_deref_frexp() {
  var a = frexp(1.5);
  let p = &a;
  var fract = p.fract;
  var exp = p.exp;
}

@compute @workgroup_size(1)
fn main() {
  deref_modf();
  no_deref_modf();
  deref_frexp();
  no_deref_frexp();
}
