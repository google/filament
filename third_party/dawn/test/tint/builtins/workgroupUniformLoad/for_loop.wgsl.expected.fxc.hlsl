[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared int a;

int tint_workgroupUniformLoad_a() {
  GroupMemoryBarrierWithGroupSync();
  int result = a;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared int b;

int tint_workgroupUniformLoad_b() {
  GroupMemoryBarrierWithGroupSync();
  int result = b;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

void foo() {
  {
    int i = 0;
    while (true) {
      int tint_symbol = i;
      int tint_symbol_1 = tint_workgroupUniformLoad_a();
      if (!((tint_symbol < tint_symbol_1))) {
        break;
      }
      {
      }
      {
        int tint_symbol_2 = i;
        int tint_symbol_3 = tint_workgroupUniformLoad_b();
        i = (tint_symbol_2 + tint_symbol_3);
      }
    }
  }
}
