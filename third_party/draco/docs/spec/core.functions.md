
## Core Functions

### LEB128

~~~~~
uint64_t LEB128() {
  result = 0;
  shift = 0;
  while(true) {
    in                                                                                UI8
    result |= (low order 7 bits of in) << shift;
    if (high order bit of in == 0)
      break;
    shift += 7;
  }
  return result;
}
~~~~~
{:.draco-syntax }


### mem_get_le16

~~~~~
uint32_t mem_get_le16(mem) {
  val = mem[1] << 8;
  val |= mem[0];
  return val;
}
~~~~~
{:.draco-syntax }


### mem_get_le24

~~~~~
uint32_t mem_get_le24(mem) {
  val = mem[2] << 16;
  val |= mem[1] << 8;
  val |= mem[0];
  return val;
}
~~~~~
{:.draco-syntax }


### mem_get_le32

~~~~~
uint32_t mem_get_le32(mem) {
  val = mem[3] << 24;
  val |= mem[2] << 16;
  val |= mem[1] << 8;
  val |= mem[0];
  return val;
}
~~~~~
{:.draco-syntax }
