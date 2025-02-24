ByteAddressBuffer sspp962805860buildInformation : register(t2);

typedef int sspp962805860buildInformation_load_ret[6];
sspp962805860buildInformation_load_ret sspp962805860buildInformation_load(uint offset) {
  int arr[6] = (int[6])0;
  {
    for(uint i = 0u; (i < 6u); i = (i + 1u)) {
      arr[i] = asint(sspp962805860buildInformation.Load((offset + (i * 4u))));
    }
  }
  return arr;
}

void main_1() {
  int orientation[6] = (int[6])0;
  int x_23[6] = sspp962805860buildInformation_load(36u);
  orientation[0] = x_23[0u];
  orientation[1] = x_23[1u];
  orientation[2] = x_23[2u];
  orientation[3] = x_23[3u];
  orientation[4] = x_23[4u];
  orientation[5] = x_23[5u];
  return;
}

void main() {
  main_1();
  return;
}
