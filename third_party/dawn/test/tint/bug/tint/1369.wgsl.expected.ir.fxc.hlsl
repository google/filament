
static bool continue_execution = true;
bool call_discard() {
  continue_execution = false;
  return true;
}

void f() {
  bool v = call_discard();
  bool also_unreachable = false;
  if (!(continue_execution)) {
    discard;
  }
}

