struct S2 {
  float3x3 m[1];
};

struct S {
  float3x3 m;
};

struct S4 {
  S s[1];
};

struct S3 {
  S s;
};


RWByteAddressBuffer buffer0 : register(u0);
RWByteAddressBuffer buffer1 : register(u1);
RWByteAddressBuffer buffer2 : register(u2);
RWByteAddressBuffer buffer3 : register(u3);
RWByteAddressBuffer buffer4 : register(u4);
RWByteAddressBuffer buffer5 : register(u5);
RWByteAddressBuffer buffer6 : register(u6);
RWByteAddressBuffer buffer7 : register(u7);
void v(uint offset, float3x3 obj) {
  buffer7.Store3((offset + 0u), asuint(obj[0u]));
  buffer7.Store3((offset + 16u), asuint(obj[1u]));
  buffer7.Store3((offset + 32u), asuint(obj[2u]));
}

void v_1(uint offset, float3x3 obj[1]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 1u)) {
        break;
      }
      v((offset + (v_3 * 48u)), obj[v_3]);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}

void v_4(uint offset, S2 obj) {
  float3x3 v_5[1] = obj.m;
  v_1((offset + 0u), v_5);
}

void v_6(uint offset, S2 obj[1]) {
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 1u)) {
        break;
      }
      S2 v_9 = obj[v_8];
      v_4((offset + (v_8 * 48u)), v_9);
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
}

void v_10(uint offset, float3x3 obj) {
  buffer6.Store3((offset + 0u), asuint(obj[0u]));
  buffer6.Store3((offset + 16u), asuint(obj[1u]));
  buffer6.Store3((offset + 32u), asuint(obj[2u]));
}

void v_11(uint offset, S obj) {
  v_10((offset + 0u), obj.m);
}

void v_12(uint offset, S obj[1]) {
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 1u)) {
        break;
      }
      S v_15 = obj[v_14];
      v_11((offset + (v_14 * 48u)), v_15);
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
}

void v_16(uint offset, float3x3 obj) {
  buffer5.Store3((offset + 0u), asuint(obj[0u]));
  buffer5.Store3((offset + 16u), asuint(obj[1u]));
  buffer5.Store3((offset + 32u), asuint(obj[2u]));
}

void v_17(uint offset, float3x3 obj[1]) {
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 1u)) {
        break;
      }
      v_16((offset + (v_19 * 48u)), obj[v_19]);
      {
        v_18 = (v_19 + 1u);
      }
      continue;
    }
  }
}

void v_20(uint offset, float3x3 obj) {
  buffer4.Store3((offset + 0u), asuint(obj[0u]));
  buffer4.Store3((offset + 16u), asuint(obj[1u]));
  buffer4.Store3((offset + 32u), asuint(obj[2u]));
}

void v_21(uint offset, S obj) {
  v_20((offset + 0u), obj.m);
}

void v_22(uint offset, S obj[1]) {
  {
    uint v_23 = 0u;
    v_23 = 0u;
    while(true) {
      uint v_24 = v_23;
      if ((v_24 >= 1u)) {
        break;
      }
      S v_25 = obj[v_24];
      v_21((offset + (v_24 * 48u)), v_25);
      {
        v_23 = (v_24 + 1u);
      }
      continue;
    }
  }
}

void v_26(uint offset, S4 obj) {
  S v_27[1] = obj.s;
  v_22((offset + 0u), v_27);
}

void v_28(uint offset, float3x3 obj) {
  buffer3.Store3((offset + 0u), asuint(obj[0u]));
  buffer3.Store3((offset + 16u), asuint(obj[1u]));
  buffer3.Store3((offset + 32u), asuint(obj[2u]));
}

void v_29(uint offset, S obj) {
  v_28((offset + 0u), obj.m);
}

void v_30(uint offset, S3 obj) {
  S v_31 = obj.s;
  v_29((offset + 0u), v_31);
}

void v_32(uint offset, float3x3 obj) {
  buffer2.Store3((offset + 0u), asuint(obj[0u]));
  buffer2.Store3((offset + 16u), asuint(obj[1u]));
  buffer2.Store3((offset + 32u), asuint(obj[2u]));
}

void v_33(uint offset, float3x3 obj[1]) {
  {
    uint v_34 = 0u;
    v_34 = 0u;
    while(true) {
      uint v_35 = v_34;
      if ((v_35 >= 1u)) {
        break;
      }
      v_32((offset + (v_35 * 48u)), obj[v_35]);
      {
        v_34 = (v_35 + 1u);
      }
      continue;
    }
  }
}

void v_36(uint offset, S2 obj) {
  float3x3 v_37[1] = obj.m;
  v_33((offset + 0u), v_37);
}

void v_38(uint offset, float3x3 obj) {
  buffer1.Store3((offset + 0u), asuint(obj[0u]));
  buffer1.Store3((offset + 16u), asuint(obj[1u]));
  buffer1.Store3((offset + 32u), asuint(obj[2u]));
}

void v_39(uint offset, S obj) {
  v_38((offset + 0u), obj.m);
}

void v_40(uint offset, float3x3 obj) {
  buffer0.Store3((offset + 0u), asuint(obj[0u]));
  buffer0.Store3((offset + 16u), asuint(obj[1u]));
  buffer0.Store3((offset + 32u), asuint(obj[2u]));
}

[numthreads(1, 1, 1)]
void main() {
  float3x3 m = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  {
    uint c = 0u;
    while(true) {
      if ((c < 3u)) {
      } else {
        break;
      }
      uint v_41 = c;
      float v_42 = float(((c * 3u) + 1u));
      float v_43 = float(((c * 3u) + 2u));
      float3 v_44 = float3(v_42, v_43, float(((c * 3u) + 3u)));
      switch(v_41) {
        case 0u:
        {
          m[0u] = v_44;
          break;
        }
        case 1u:
        {
          m[1u] = v_44;
          break;
        }
        case 2u:
        {
          m[2u] = v_44;
          break;
        }
        default:
        {
          break;
        }
      }
      {
        c = (c + 1u);
      }
      continue;
    }
  }
  float3x3 a = m;
  v_40(0u, a);
  S a_1 = {m};
  v_39(0u, a_1);
  float3x3 v_45[1] = {m};
  S2 a_2 = {v_45};
  v_36(0u, a_2);
  S v_46 = {m};
  S3 a_3 = {v_46};
  v_30(0u, a_3);
  S v_47 = {m};
  S v_48[1] = {v_47};
  S4 a_4 = {v_48};
  v_26(0u, a_4);
  float3x3 a_5[1] = {m};
  v_17(0u, a_5);
  S v_49 = {m};
  S a_6[1] = {v_49};
  v_12(0u, a_6);
  float3x3 v_50[1] = {m};
  S2 v_51 = {v_50};
  S2 a_7[1] = {v_51};
  v_6(0u, a_7);
}

