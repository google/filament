# dndxc

## Help System

This README.md file also acts as the entry point into the help system.

Press F1 to get contextual help.

## Help Topics

### Descriptor

Attributes: Name, ResName, CounterName, Kind (one of UAV,SRV,CBV)
Attributes from D3D12_UNORDERED_ACCESS_VIEW_DESC and D3D12_SHADER_RESOURCE_VIEW_DESC:
- Format
- Dimension

When Dimension is D3D12_SRV_DIMENSION_BUFFER:
- FirstElement, Flags (empty or RAW), NumElements, StructureByteStride

When Dimension is D3D12_UAV_DIMENSION_BUFFER:
- FirstElement, NumElements, StructuredByteStride, CounterOffsetInBytes, Flags
Flags can be emptyy or RAW. If RAW, Format is forced to DXGI_FORMAT_R32_TYPELESS.

When Dimension is D3D12_UAV_DIMENSION_TEXTURE1D:
- MipSlice

When Dimension is D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
- MipSlice, FirstArraySlice, ArraySize

When Dimension is D3D12_UAV_DIMENSION_TEXTURE2D:
- MipSlice, PlaneSlice

When Dimension is D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
- MipSlice, FirstArraySlice, ArraySize, PlaneSlice

When Dimension is D3D12_UAV_DIMENSION_TEXTURE3D:
- MipSlice, FirstWSlice, WSize

If either of Name or ResName is assigned and the other isn't, they default to the assigned name.

### DescriptorHeap

Attributes: Name, Flags, NodeMask, NumDescriptors, Type
Flags defaults to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE.
Type is one of (CBV_SRV_UAV,SAMPLER,RTV,DSV) and defaults to D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV.
When Type is D3D12_DESCRIPTOR_HEAP_TYPE_RTV, Flags defaults to NONE.

### Resource

Attributes: Name, Init, ReadBack, HeapType, CPUPageProperty, MemoryPoolPreference, CreationNodeMask, VisibleNodeMask
Attributes from D3D12_RESOURCE_DESC:
- Dimension
- Alignment
- Width
- Height
- DepthOrArraySize
- MipLevels
- Format
- SampleCount, SampleQual
- Layout
- Flags

Other attributes:
- HeapFlags
- InitialResourceState
- TransitionTo
- Topology

Some values are fixed depending on the value of Dimension:
  if (pResource->Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
    pResource->Desc.Height = 1;
    pResource->Desc.DepthOrArraySize = 1;
    pResource->Desc.MipLevels = 1;
    pResource->Desc.Format = DXGI_FORMAT_UNKNOWN;
    pResource->Desc.SampleDesc.Count = 1;
    pResource->Desc.SampleDesc.Quality = 0;
    pResource->Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  }
  if (pResource->Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D) {
    if (pResource->Desc.Height == 0) pResource->Desc.Height = 1;
    if (pResource->Desc.DepthOrArraySize == 0) pResource->Desc.DepthOrArraySize = 1;
    if (pResource->Desc.SampleDesc.Count == 0) pResource->Desc.SampleDesc.Count = 1;
  }
  if (pResource->Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D) {
    if (pResource->Desc.DepthOrArraySize == 0) pResource->Desc.DepthOrArraySize = 1;
    if (pResource->Desc.SampleDesc.Count == 0 ) pResource->Desc.SampleDesc.Count = 1;
  }

The contents of the element initialize the values within.

The following characters are ignored: '{', '}', ',', '\w'

The suffix of each value determines how it's interpreted: h, l, u, i, f

Friendly names include nan, inf, +inf, -inf, -denorm, denorm

### RootSignature

This element defines the root signature string.

See https://msdn.microsoft.com/en-us/library/windows/desktop/dn913202(v=vs.85).aspx for a reference.

Example:
RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT),CBV(b0),SRV(t0),UAV(u0),DescriptorTable(CBV(b1),SRV(t1,numDescriptors=2),UAV(u1))

### RootValue

This element defines a single value in a RootValues collection.

Attributes: ResName, HeapName, Index

HeapName names the heap in which the resource is placed, ResName the name, and Index an index into that resource table.
HeapName and ResName are exclusive. HeapName refers to a descriptor table (allocated on its own heap) and ResName refers to a Resource.

Remember: SRV or UAV root descriptors can only be Raw or Structured buffers.

### RootValues

This element defines the root values to be used in a ShaderOp.

Element: RootValue

### ShaderOp

This elements define a single Draw or Dispatch operation.

Attributes: Name, CS, VS, PS, DispatchX, DispatchY, DispatchZ, TopologyType
Elements: InputElements, Shader, RootSignature, RenderTargets, Resource, DescriptorHeap, RootValues

### ShaderOpSet

This element defines a set of shader operations. It's useful when bundling
multiple shaders in a single file.

Elements: ShaderOp
