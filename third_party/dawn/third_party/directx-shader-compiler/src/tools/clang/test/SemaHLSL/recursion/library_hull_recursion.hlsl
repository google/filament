// RUN: %dxc -T lib_6_5 -verify %s


// no error, because this function isn't exported. 
// it's reachable from main, but recurse is detected first before recurse2
void recurse2(inout float4 f, float a) {
  if (a > 0) {
    recurse2(f, a);
  }
  f -= abs(f+a);
}

void recurse(inout float4 f, float a) /* expected-error{{recursive functions are not allowed: function 'main' calls recursive function 'recurse'}}
                                         expected-note{{recursive function located here:}} */
{
    if (a > 1) {
      recurse(f, a-1);
      recurse2(f, a-1);
    }
    f += abs(f+a);
}

struct HSPerPatchData
{
  float edges[3] : SV_TessFactor;
  float inside   : SV_InsideTessFactor;
};

HSPerPatchData HSPerPatchFunc1() /* expected-error{{recursive functions are not allowed: function 'HSPerPatchFunc1' calls recursive function 'HSPerPatchFunc1'}}
                                    expected-note{{recursive function located here:}} */
{
  HSPerPatchData d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.inside = -8;
  HSPerPatchFunc1();
  return d;
}

[shader("hull")]
[patchconstantfunc("HSPerPatchFunc1")]
[outputtopology("point")]
[outputcontrolpoints(1)]
float4 main(float a : A, float b:B) : SV_TARGET
{
  float4 f = b;
  recurse(f, a);
  return f;
}

