// RUN: %dxc -fcgl -HV 2018 -T ps_6_0 %s | FileCheck %s
// RUN: %dxc -fcgl -HV 2021 -T ps_6_0 %s | FileCheck %s

// Make sure the select() built-in works for sampler objects



// SamplerState objects to use in selects
SamplerState gSS1;
SamplerState gSS2;
SamplerComparisonState gSCS1;
SamplerComparisonState gSCS2;

Texture1D<float4> gTX1D;

// A very slightly convoluted way to get a certain true result no matter the parameter
bool getCond(int i) {
  return i > 0 || i <= 0;
}

float4 main(int2 i : I, float4 pos : POS, float cmp :CMP) : SV_Target {
  // Test ?: initializations

  // CHECK: [[gSS1A:%[0-9]*]] = load %struct.SamplerState, %struct.SamplerState* @"\01?gSS1@@3USamplerState@@A"
  // CHECK: store %struct.SamplerState [[gSS1A]], %struct.SamplerState* %lSS0
  // CHECK-NEXT: br
  // CHECK: [[gSS2A:%[0-9]*]] = load %struct.SamplerState, %struct.SamplerState* @"\01?gSS2@@3USamplerState@@A"
  // CHECK: store %struct.SamplerState [[gSS2A]], %struct.SamplerState* %lSS0
  // CHECK-NEXT: br
  SamplerState lSS0 = getCond(i.x) ? gSS1 : gSS2;

  // CHECK: [[gSCS1A:%[0-9]*]] = load %struct.SamplerComparisonState, %struct.SamplerComparisonState* @"\01?gSCS1@@3USamplerComparisonState@@A"
  // CHECK: store %struct.SamplerComparisonState [[gSCS1A]], %struct.SamplerComparisonState* %lSCS0
  // CHECK-NEXT: br
  // CHECK: [[gSCS2A:%[0-9]*]] = load %struct.SamplerComparisonState, %struct.SamplerComparisonState* @"\01?gSCS2@@3USamplerComparisonState@@A"
  // CHECK: store %struct.SamplerComparisonState [[gSCS2A]], %struct.SamplerComparisonState* %lSCS0
  // CHECK-NEXT: br
  SamplerComparisonState lSCS0 = getCond(i.x) ? gSCS1 : gSCS2;


  // Test select() initializations

  // CHECK-NOT: br
  // CHECK: [[gSS1B:%[0-9]*]] = load %struct.SamplerState, %struct.SamplerState* @"\01?gSS1@@3USamplerState@@A"
  // CHECK: [[gSS1CHB:%[0-9]*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.SamplerState)"(i32 0, %struct.SamplerState [[gSS1B]])
  // CHECK: [[gSS1CAB:%[0-9]*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.SamplerState)"(i32 {{[0-9]*}}, %dx.types.Handle [[gSS1CHB]]

  // CHECK: [[gSS2B:%[0-9]*]] = load %struct.SamplerState, %struct.SamplerState* @"\01?gSS2@@3USamplerState@@A"
  // CHECK: [[gSS2CHB:%[0-9]*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.SamplerState)"(i32 0, %struct.SamplerState [[gSS2B]])
  // CHECK: [[gSS2CAB:%[0-9]*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.SamplerState)"(i32 {{[0-9]*}}, %dx.types.Handle [[gSS2CHB]]

  // CHECK: call %dx.types.Handle @"dx.hl.op..%dx.types.Handle (i32, i1, %dx.types.Handle, %dx.types.Handle)"(i32 {{[0-9]*}}, i1 %{{[0-9a-zA-Z_]*}}, %dx.types.Handle [[gSS1CAB]], %dx.types.Handle [[gSS2CAB]])

  SamplerState lSS1 = select(getCond(i.x), gSS1, gSS2);

  // CHECK: [[gSCS1B:%[0-9]*]] = load %struct.SamplerComparisonState, %struct.SamplerComparisonState* @"\01?gSCS1@@3USamplerComparisonState@@A"
  // CHECK: [[gSCS1CHB:%[0-9]*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.SamplerComparisonState)"(i32 0, %struct.SamplerComparisonState [[gSCS1B]])
  // CHECK: [[gSCS1CAB:%[0-9]*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.SamplerComparisonState)"(i32 {{[0-9]*}}, %dx.types.Handle [[gSCS1CHB]]

  // CHECK: [[gSCS2B:%[0-9]*]] = load %struct.SamplerComparisonState, %struct.SamplerComparisonState* @"\01?gSCS2@@3USamplerComparisonState@@A"
  // CHECK: [[gSCS2CHB:%[0-9]*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.SamplerComparisonState)"(i32 0, %struct.SamplerComparisonState [[gSCS2B]])
  // CHECK: [[gSCS2CAB:%[0-9]*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.SamplerComparisonState)"(i32 {{[0-9]*}}, %dx.types.Handle [[gSCS2CHB]]

  // CHECK: call %dx.types.Handle @"dx.hl.op..%dx.types.Handle (i32, i1, %dx.types.Handle, %dx.types.Handle)"(i32 {{[0-9]*}}, i1 %{{[0-9a-zA-Z_]*}}, %dx.types.Handle [[gSCS1CAB]], %dx.types.Handle [[gSCS2CAB]])

  SamplerComparisonState lSCS1 = select(getCond(i.x), gSCS1, gSCS2);

  // Assign post initialization, uses a slightly different code path

  // Test assignment using ?:

  // CHECK: br
  // CHECK: [[gSS2C:%[0-9]*]] = load %struct.SamplerState, %struct.SamplerState* @"\01?gSS2@@3USamplerState@@A"
  // CHECK: store %struct.SamplerState [[gSS2C]], %struct.SamplerState* %lSS0
  // CHECK: br
  // CHECK: [[lSS0C:%[0-9]*]] = load %struct.SamplerState, %struct.SamplerState* %lSS0
  // CHECK: store %struct.SamplerState [[lSS0C]], %struct.SamplerState* %lSS0
  // CHECK: br
  lSS0 = getCond(i.y) ? gSS2 : lSS0;

  // CHECK: [[gSCS2C:%[0-9]*]] = load %struct.SamplerComparisonState, %struct.SamplerComparisonState* @"\01?gSCS2@@3USamplerComparisonState@@A"
  // CHECK: store %struct.SamplerComparisonState [[gSCS2C]], %struct.SamplerComparisonState* %lSCS0
  // CHECK: br
  // CHECK: [[lSCS0C:%[0-9]*]] = load %struct.SamplerComparisonState, %struct.SamplerComparisonState* %lSCS0
  // CHECK: store %struct.SamplerComparisonState [[lSCS0C]], %struct.SamplerComparisonState* %lSCS0
  // CHECK: br
  lSCS0 = getCond(i.y) ? gSCS2 : lSCS0;

  // Test assignment using select()

  // CHECK-NOT: br
  // CHECK: [[gSS2D:%[0-9]*]] = load %struct.SamplerState, %struct.SamplerState* @"\01?gSS2@@3USamplerState@@A"
  // CHECK: [[gSS2CHD:%[0-9]*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.SamplerState)"(i32 0, %struct.SamplerState [[gSS2D]])
  // CHECK: [[gSS2CAD:%[0-9]*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.SamplerState)"(i32 {{[0-9]*}}, %dx.types.Handle [[gSS2CHD]]

  // CHECK: [[lSS0D:%[0-9]*]] = load %struct.SamplerState, %struct.SamplerState* %lSS1
  // CHECK: [[lSS0CHD:%[0-9]*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.SamplerState)"(i32 0, %struct.SamplerState [[lSS0D]])
  // CHECK: [[lSS0CAD:%[0-9]*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.SamplerState)"(i32 {{[0-9]*}}, %dx.types.Handle [[lSS0CHD]]

  // CHECK: call %dx.types.Handle @"dx.hl.op..%dx.types.Handle (i32, i1, %dx.types.Handle, %dx.types.Handle)"(i32 {{[0-9]*}}, i1 %{{[0-9a-zA-Z_]*}}, %dx.types.Handle [[gSS2CAD]], %dx.types.Handle [[lSS0CAD]])

  lSS1 = select(getCond(i.y), gSS2, lSS1);

  // CHECK: [[gSCS2D:%[0-9]*]] = load %struct.SamplerComparisonState, %struct.SamplerComparisonState* @"\01?gSCS2@@3USamplerComparisonState@@A"
  // CHECK: [[gSCS2CHD:%[0-9]*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.SamplerComparisonState)"(i32 0, %struct.SamplerComparisonState [[gSCS2D]])
  // CHECK: [[gSCS2CAD:%[0-9]*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.SamplerComparisonState)"(i32 {{[0-9]*}}, %dx.types.Handle [[gSCS2CHD]]

  // CHECK: [[lSCS0D:%[0-9]*]] = load %struct.SamplerComparisonState, %struct.SamplerComparisonState* %lSCS1
  // CHECK: [[lSCS0CHD:%[0-9]*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.SamplerComparisonState)"(i32 0, %struct.SamplerComparisonState [[lSCS0D]])
  // CHECK: [[lSCS0CAD:%[0-9]*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.SamplerComparisonState)"(i32 {{[0-9]*}}, %dx.types.Handle [[lSCS0CHD]]

  // CHECK: call %dx.types.Handle @"dx.hl.op..%dx.types.Handle (i32, i1, %dx.types.Handle, %dx.types.Handle)"(i32 {{[0-9]*}}, i1 %{{[0-9a-zA-Z_]*}}, %dx.types.Handle [[gSCS2CAD]], %dx.types.Handle [[lSCS0CAD]])
  lSCS1 = select(getCond(i.y), gSCS2, lSCS1);

  // Make some trivial use of these so the shader is just slightly
  // more representative of actual useful shaders
  float4 l = 0;
  l += gTX1D.Sample(lSS0, pos.x);
  l *= gTX1D.SampleCmp(lSCS0, pos.x, cmp);
  l += gTX1D.Sample(lSS1, pos.x);
  l *= gTX1D.SampleCmp(lSCS1, pos.x, cmp);

  return l;
}
