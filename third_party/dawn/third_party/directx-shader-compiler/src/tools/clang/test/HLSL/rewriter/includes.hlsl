#include "toinclude.hlsl"
#include "..\toinclude2.hlsl"
#include "for_includes_test\toinclude3.hlsl"

int func1(int b){
  return includedFunc(b);
}

int func2(int d){
  return includedFunc2(d);
}

int func3(int f){
  return includedFunc3(f);
}
