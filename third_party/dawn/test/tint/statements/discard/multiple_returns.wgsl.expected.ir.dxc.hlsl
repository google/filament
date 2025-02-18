
RWByteAddressBuffer non_uniform_global : register(u0);
RWByteAddressBuffer output : register(u1);
void main() {
  if ((asint(non_uniform_global.Load(0u)) < int(0))) {
    discard;
  }
  output.Store(0u, asuint(ddx(1.0f)));
  if ((asfloat(output.Load(0u)) < 0.0f)) {
    int i = int(0);
    {
      uint2 tint_loop_idx = (0u).xx;
      while(true) {
        if (all((tint_loop_idx == (4294967295u).xx))) {
          break;
        }
        float v = asfloat(output.Load(0u));
        if ((v > float(i))) {
          output.Store(0u, asuint(float(i)));
          return;
        }
        {
          uint tint_low_inc = (tint_loop_idx.x + 1u);
          tint_loop_idx.x = tint_low_inc;
          uint tint_carry = uint((tint_low_inc == 0u));
          tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
          i = (i + int(1));
          if ((i == int(5))) { break; }
        }
        continue;
      }
    }
    return;
  }
}

