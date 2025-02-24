bool call_discard() {
  if (true) {
    discard;
    return true;
  }
  bool unused;
  return unused;
}

void f() {
  bool v = call_discard();
  bool also_unreachable = false;
  return;
}
