// RUN: %dxc -Tlib_6_3   -verify  %s
// RUN: %dxc -Tcs_6_0   -verify  %s

// Verify that the first arg determines the overload and the others can be what they will

groupshared int      resGI[256];
groupshared uint64_t resGI64[256];
RWBuffer<int>      resBI;
RWBuffer<uint64_t> resBI64;

RWByteAddressBuffer Rres;

[shader("compute")]
[numthreads(1,1,1)]
void main( uint3 gtid : SV_GroupThreadID)
{
  uint a = gtid.x;
  uint b = gtid.y;
  uint c = gtid.z;
  resGI[a] = a;
  resGI64[a] = a;
  resBI[a] = a;
  resBI64[a] = a;

  float fv = b - c;
  float fv2 = b + c;
  float ofv = 0;
  int iv = b / c;
  int iv2 = b * c;
  int oiv = 0;
  uint64_t bb = b;
  uint64_t cc = c;
  uint64_t lv = bb * cc;
  uint64_t lv2 = bb / cc;
  uint64_t olv = 0;

  InterlockedCompareStoreFloatBitwise( resBI[a], iv, iv2 ); // expected-error{{no matching function for call to 'InterlockedCompareStoreFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'int' to 'float &' for 1st argument}}
  InterlockedCompareStoreFloatBitwise( resBI64[a], lv, lv2); // expected-error{{no matching function for call to 'InterlockedCompareStoreFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'unsigned long long' to 'float &' for 1st argument}}
  InterlockedCompareStoreFloatBitwise( resGI[a], iv, iv2 ); // expected-error{{no matching function for call to 'InterlockedCompareStoreFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'int' to 'float &' for 1st argument}}
  InterlockedCompareStoreFloatBitwise( resGI64[a], lv, lv2); // expected-error{{no matching function for call to 'InterlockedCompareStoreFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'uint64_t' to 'float &' for 1st argument}}

  InterlockedCompareStoreFloatBitwise( resBI[a], fv, fv2 ); // expected-error{{no matching function for call to 'InterlockedCompareStoreFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'int' to 'float &' for 1st argument}}
  InterlockedCompareStoreFloatBitwise( resBI64[a], fv, fv2 ); // expected-error{{no matching function for call to 'InterlockedCompareStoreFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'unsigned long long' to 'float &' for 1st argument}}
  InterlockedCompareStoreFloatBitwise( resGI[a], fv, fv2 ); // expected-error{{no matching function for call to 'InterlockedCompareStoreFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'int' to 'float &' for 1st argument}}
  InterlockedCompareStoreFloatBitwise( resGI64[a], fv, fv2 ); // expected-error{{no matching function for call to 'InterlockedCompareStoreFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'uint64_t' to 'float &' for 1st argument}}

  InterlockedCompareExchangeFloatBitwise( resBI[a], iv, iv2, oiv ); // expected-error{{no matching function for call to 'InterlockedCompareExchangeFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'int' to 'float &' for 1st argument}}
  InterlockedCompareExchangeFloatBitwise( resBI64[a], lv, lv2, olv); // expected-error{{no matching function for call to 'InterlockedCompareExchangeFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'unsigned long long' to 'float &' for 1st argument}}
  InterlockedCompareExchangeFloatBitwise( resGI[a], iv, iv2, oiv ); // expected-error {{no matching function for call to 'InterlockedCompareExchangeFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'int' to 'float &' for 1st argument}}
  InterlockedCompareExchangeFloatBitwise( resGI64[a], lv, lv2, olv); // expected-error{{no matching function for call to 'InterlockedCompareExchangeFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'uint64_t' to 'float &' for 1st argument}}

  InterlockedCompareExchangeFloatBitwise( resBI[a], fv, fv2, ofv ); // expected-error{{no matching function for call to 'InterlockedCompareExchangeFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'int' to 'float &' for 1st argument}}
  InterlockedCompareExchangeFloatBitwise( resBI64[a], fv, fv2, ofv ); // expected-error{{no matching function for call to 'InterlockedCompareExchangeFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'unsigned long long' to 'float &' for 1st argument}}
  InterlockedCompareExchangeFloatBitwise( resGI[a], fv, fv2, ofv ); // expected-error{{no matching function for call to 'InterlockedCompareExchangeFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'int' to 'float &' for 1st argument}}
  InterlockedCompareExchangeFloatBitwise( resGI64[a], fv, fv2, ofv ); // expected-error{{no matching function for call to 'InterlockedCompareExchangeFloatBitwise'}} expected-note{{candidate function not viable: no known conversion from 'uint64_t' to 'float &' for 1st argument}}

}
