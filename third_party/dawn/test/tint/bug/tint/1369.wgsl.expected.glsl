#version 310 es
precision highp float;
precision highp int;

bool continue_execution = true;
bool call_discard() {
  continue_execution = false;
  return true;
}
void main() {
  bool v = call_discard();
  bool also_unreachable = false;
  if (!(continue_execution)) {
    discard;
  }
}
