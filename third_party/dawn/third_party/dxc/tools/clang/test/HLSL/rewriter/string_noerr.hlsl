// RUN: %clang_cc1 -fsyntax-only -Wno-unused-value -ffreestanding -verify %s

// expected-no-diagnostics

#if 0

string s_global = "string";
string s_global_concat = "string" "with "
  "broken up"
  "parts";

void hello_here(string message, string s, float f) {
  printf(s);
  printf(message);
  printf("%f", f);
}

float4 main() : SV_Target0{
  float4 cp4_local = cp4;
  int i = 0;
  cp4_local.x = a("hi" "bob");
  printf("hi mom", 1, 2, 3);
  hello_here("a", "b", 1);
  return cp4_local;
}

#endif
