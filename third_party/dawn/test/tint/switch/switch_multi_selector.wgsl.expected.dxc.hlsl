[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void a() {
  int a_1 = 0;
  switch(a_1) {
    case 0:
    case 2:
    case 4: {
      break;
    }
    case 1:
    default: {
      return;
      break;
    }
  }
}
