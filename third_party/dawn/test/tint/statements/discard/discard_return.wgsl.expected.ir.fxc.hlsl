
static bool continue_execution = true;
void f() {
  continue_execution = false;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

