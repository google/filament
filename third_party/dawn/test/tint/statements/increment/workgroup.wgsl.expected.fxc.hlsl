[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared int i;

void main() {
  i = (i + 1);
}
