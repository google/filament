static bool tint_discarded = false;

bool call_discard() {
  tint_discarded = true;
  return true;
}

void f() {
  bool v = call_discard();
  bool also_unreachable = false;
  if (tint_discarded) {
    discard;
  }
  return;
}
