// RUN: %dxc -Tlib_6_3 -verify %s
// RUN: %dxc -Tcs_6_0 -verify %s

RWByteAddressBuffer NonGCBuf;
globallycoherent RWByteAddressBuffer GCBuf;

RWByteAddressBuffer NonGCBufArr[2];
globallycoherent RWByteAddressBuffer GCBufArr[2];

RWByteAddressBuffer NonGCBufMultiArr[2][2];
globallycoherent RWByteAddressBuffer GCBufMultiArr[2][2];

RWByteAddressBuffer getNonGCBuf() {
  return NonGCBuf;
}

globallycoherent RWByteAddressBuffer getGCBuf() { 
  return GCBuf;
}

RWByteAddressBuffer getNonGCBufArr() {
  return NonGCBufArr[0];
}

globallycoherent RWByteAddressBuffer getGCBufArr() { 
  return GCBufArr[0];
}

RWByteAddressBuffer getNonGCBufMultiArr() {
  return NonGCBufMultiArr[0][0];
}

globallycoherent RWByteAddressBuffer getGCBufMultiArr() { 
  return GCBufMultiArr[0][0];
}

RWByteAddressBuffer getNonGCGCBuf() {
  return GCBuf; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses globallycoherent annotation}}
}

globallycoherent RWByteAddressBuffer getGCNonGCBuf() {
  return NonGCBuf; // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' adds globallycoherent annotation}}
}

RWByteAddressBuffer getNonGCGCBufArr() {
  return GCBufArr[0]; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses globallycoherent annotation}}
}

globallycoherent RWByteAddressBuffer getGCNonGCBufArr() {
  return NonGCBufArr[0]; // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' adds globallycoherent annotation}}
}

RWByteAddressBuffer getNonGCGCBufMultiArr() {
  return GCBufMultiArr[0][0]; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses globallycoherent annotation}}
}

globallycoherent RWByteAddressBuffer getGCNonGCBufMultiArr() {
  return NonGCBufMultiArr[0][0]; // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' adds globallycoherent annotation}}
}

void NonGCStore(RWByteAddressBuffer Buf) {
  Buf.Store(0, 0);
}

void GCStore(globallycoherent RWByteAddressBuffer Buf) {
  Buf.Store(0, 0);
}


void getNonGCBufPAram(inout globallycoherent RWByteAddressBuffer PGCBuf) {
  PGCBuf = NonGCBuf; // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer __restrict' adds globallycoherent annotation}}
}

static globallycoherent RWByteAddressBuffer SGCBufArr[2] = NonGCBufArr; // expected-warning{{implicit conversion from 'RWByteAddressBuffer [2]' to 'globallycoherent RWByteAddressBuffer [2]' adds globallycoherent annotation}}
static globallycoherent RWByteAddressBuffer SGCBufMultiArr0[2] = NonGCBufMultiArr[0]; // expected-warning{{implicit conversion from 'RWByteAddressBuffer [2]' to 'globallycoherent RWByteAddressBuffer [2]' adds globallycoherent annotation}}
static globallycoherent RWByteAddressBuffer SGCBufMultiArr1[2][2] = NonGCBufMultiArr; // expected-warning{{implicit conversion from 'RWByteAddressBuffer [2][2]' to 'globallycoherent RWByteAddressBuffer [2][2]' adds globallycoherent annotation}}

void getNonGCBufArrParam(inout globallycoherent RWByteAddressBuffer PGCBufArr[2]) {
  PGCBufArr = NonGCBufArr; // expected-warning{{implicit conversion from 'RWByteAddressBuffer [2]' to 'globallycoherent RWByteAddressBuffer __restrict[2]' adds globallycoherent annotation}}
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
  NonGCStore(NonGCBuf); // No diagnostic
  GCStore(NonGCBuf); // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' adds globallycoherent annotation}}
  NonGCStore(GCBuf); // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses globallycoherent annotation}}
  GCStore(GCBuf); // No diagnostic

  RWByteAddressBuffer NonGCCopyNonGC = NonGCBuf; // No diagnostic
  RWByteAddressBuffer NonGCCopyGC = GCBuf; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses globallycoherent annotation}}

  globallycoherent RWByteAddressBuffer GCCopyNonGC = NonGCBuf; // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' adds globallycoherent annotation}}
  globallycoherent RWByteAddressBuffer GCCopyGC = GCBuf; // No diagnostic

  globallycoherent RWByteAddressBuffer GCCopyNonGCReturn = getNonGCBuf(); // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' adds globallycoherent annotation}}

  RWByteAddressBuffer NonGCCopyGCReturn = getGCBuf(); // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses globallycoherent annotation}}

  RWByteAddressBuffer NonGCCopyNonGC0 = NonGCBufArr[0]; // No diagnostic
  RWByteAddressBuffer NonGCCopyGC0 = GCBufArr[0]; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses globallycoherent annotation}}

  globallycoherent RWByteAddressBuffer GCCopyNonGC0 = NonGCBufArr[0]; // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' adds globallycoherent annotation}}
  globallycoherent RWByteAddressBuffer GCCopyGC0 = GCBufArr[0]; // No diagnostic
}
