// RUN: %dxc -Tlib_6_9 -verify %s

RWByteAddressBuffer NonRCBuf;
reordercoherent RWByteAddressBuffer RCBuf;

RWByteAddressBuffer NonRCBufArr[2];
reordercoherent RWByteAddressBuffer RCBufArr[2];

RWByteAddressBuffer NonRCBufMultiArr[2][2];
reordercoherent RWByteAddressBuffer RCBufMultiArr[2][2];

RWByteAddressBuffer getNonRCBuf() {
  return NonRCBuf;
}

reordercoherent RWByteAddressBuffer getRCBuf() {
  return RCBuf;
}

RWByteAddressBuffer getNonRCBufArr() {
  return NonRCBufArr[0];
}

reordercoherent RWByteAddressBuffer getRCBufArr() {
  return RCBufArr[0];
}

RWByteAddressBuffer getNonRCBufMultiArr() {
  return NonRCBufMultiArr[0][0];
}

reordercoherent RWByteAddressBuffer getRCBufMultiArr() {
  return RCBufMultiArr[0][0];
}

RWByteAddressBuffer getNonGCRCBuf() {
  return RCBuf; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses reordercoherent annotation}}
}

reordercoherent RWByteAddressBuffer getGCNonRCBuf() {
  return NonRCBuf; // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' adds reordercoherent annotation}}
}

RWByteAddressBuffer getNonGCRCBufArr() {
  return RCBufArr[0]; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses reordercoherent annotation}}
}

reordercoherent RWByteAddressBuffer getGCNonRCBufArr() {
  return NonRCBufArr[0]; // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' adds reordercoherent annotation}}
}

RWByteAddressBuffer getNonGCRCBufMultiArr() {
  return RCBufMultiArr[0][0]; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses reordercoherent annotation}}
}

reordercoherent RWByteAddressBuffer getGCNonRCBufMultiArr() {
  return NonRCBufMultiArr[0][0]; // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' adds reordercoherent annotation}}
}

void NonGCStore(RWByteAddressBuffer Buf) {
  Buf.Store(0, 0);
}

void GCStore(reordercoherent RWByteAddressBuffer Buf) {
  Buf.Store(0, 0);
}

void getNonRCBufPAram(inout reordercoherent RWByteAddressBuffer PRCBuf) {
  PRCBuf = NonRCBuf; // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer __restrict' adds reordercoherent annotation}}
}

static reordercoherent RWByteAddressBuffer SRCBufArr[2] = NonRCBufArr;               // expected-warning{{implicit conversion from 'RWByteAddressBuffer [2]' to 'reordercoherent RWByteAddressBuffer [2]' adds reordercoherent annotation}}
static reordercoherent RWByteAddressBuffer SRCBufMultiArr0[2] = NonRCBufMultiArr[0]; // expected-warning{{implicit conversion from 'RWByteAddressBuffer [2]' to 'reordercoherent RWByteAddressBuffer [2]' adds reordercoherent annotation}}
static reordercoherent RWByteAddressBuffer SRCBufMultiArr1[2][2] = NonRCBufMultiArr; // expected-warning{{implicit conversion from 'RWByteAddressBuffer [2][2]' to 'reordercoherent RWByteAddressBuffer [2][2]' adds reordercoherent annotation}}

void getNonRCBufArrParam(inout reordercoherent RWByteAddressBuffer PRCBufArr[2]) {
  PRCBufArr = NonRCBufArr; // expected-warning{{implicit conversion from 'RWByteAddressBuffer [2]' to 'reordercoherent RWByteAddressBuffer __restrict[2]' adds reordercoherent annotation}}
}

[shader("raygeneration")] void main() {
  NonGCStore(NonRCBuf); // No diagnostic
  GCStore(NonRCBuf);    // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' adds reordercoherent annotation}}
  NonGCStore(RCBuf);    // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses reordercoherent annotation}}
  GCStore(RCBuf);       // No diagnostic

  RWByteAddressBuffer NonGCCopyNonGC = NonRCBuf; // No diagnostic
  RWByteAddressBuffer NonGCCopyGC = RCBuf;       // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses reordercoherent annotation}}

  reordercoherent RWByteAddressBuffer GCCopyNonGC = NonRCBuf; // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' adds reordercoherent annotation}}
  reordercoherent RWByteAddressBuffer GCCopyGC = RCBuf;       // No diagnostic

  reordercoherent RWByteAddressBuffer GCCopyNonGCReturn = getNonRCBuf(); // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' adds reordercoherent annotation}}

  RWByteAddressBuffer NonGCCopyGCReturn = getRCBuf(); // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses reordercoherent annotation}}

  RWByteAddressBuffer NonGCCopyNonGC0 = NonRCBufArr[0]; // No diagnostic
  RWByteAddressBuffer NonGCCopyGC0 = RCBufArr[0];       // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'RWByteAddressBuffer' loses reordercoherent annotation}}

  reordercoherent RWByteAddressBuffer GCCopyNonGC0 = NonRCBufArr[0]; // expected-warning{{implicit conversion from 'RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' adds reordercoherent annotation}}
  reordercoherent RWByteAddressBuffer GCCopyGC0 = RCBufArr[0];       // No diagnostic
}
