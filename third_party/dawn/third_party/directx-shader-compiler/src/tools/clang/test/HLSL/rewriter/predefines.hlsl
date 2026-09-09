
#define myDefine2 21

int func_uses_defines(int a)
{
  int b = myDefine2;
  return a + myDefine + b + myDefine3 + myDefine4;
}