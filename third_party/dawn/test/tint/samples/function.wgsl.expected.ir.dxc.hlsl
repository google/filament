
float main() {
  return 0.40000000596046447754f;
}

[numthreads(2, 1, 1)]
void ep() {
  float a = main();
}

