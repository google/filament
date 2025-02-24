
groupshared int i;
void main() {
  i = (i - int(1));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

