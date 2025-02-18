
static bool continue_execution = true;
void f(bool cond) {
  if (cond) {
    continue_execution = false;
    return;
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

