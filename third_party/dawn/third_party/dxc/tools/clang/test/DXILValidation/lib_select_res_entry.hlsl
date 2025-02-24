
void test( uint cond);

[shader("pixel")]
float main(uint c : C) : SV_Target {
   test(c);
   return 1;
}