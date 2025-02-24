[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void main() {
  int b = 2;
  int c = (b - -(b));
}
