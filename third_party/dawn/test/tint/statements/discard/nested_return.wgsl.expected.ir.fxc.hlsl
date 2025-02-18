
RWByteAddressBuffer non_uniform_global : register(u0);
static bool continue_execution = true;
void main() {
  if ((asint(non_uniform_global.Load(0u)) < int(0))) {
    continue_execution = false;
  }
  if (!(continue_execution)) {
    discard;
  }
}

