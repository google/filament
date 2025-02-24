
[numthreads(1, 1, 1)]
void main() {
  bool a = true;
  bool v = ((false) ? (true) : ((a & true)));
}

