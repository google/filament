
bool call_discard() {
  discard;
  return true;
}

void f() {
  bool v = call_discard();
  bool also_unreachable = false;
}

