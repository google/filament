fn call_discard() -> bool {
  discard;
  return true;
}

@fragment
fn f() {
  var v = call_discard();
  var also_unreachable : bool;
}
