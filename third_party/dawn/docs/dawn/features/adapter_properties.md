# Adapter Properties

## Memory Heaps

`wgpu::FeatureName::AdapterPropertiesMemoryHeaps` allows querying memory heap information from the adapter.

`wgpu::AdapterPropertiesMemoryHeaps` may be chained on `wgpu::AdapterInfo` in a call to `wgpu::Adapter::GetInfo` or `wgpu::Device::GetAdapterInfo` in order to query information about the memory heaps on the adapter.
The implementation will write out the number of memory heaps and information about each heap.

If `wgpu::FeatureName::AdapterPropertiesMemoryHeaps` is not available, the struct will not be populated.

Adds `wgpu::HeapProperty` which is a bitmask describing the type of memory a heap is. Valid bits:
- DeviceLocal
- HostVisible
- HostCoherent
- HostUncached
- HostCached

Note that both HostUncached and HostCached may be set if a heap can allocate pages with either cache property.

Adds `wgpu::MemoryHeapInfo` which is a struct describing a memory heap.
```
struct MemoryHeapInfo {
    HeapProperty properties;
    uint64_t size;
};
```

`wgpu::MemoryHeapInfo::size` is the size that should be allocated out of this heap. Allocating more than this may result in poor performance or may deterministically run out of memory.


## D3D

`wgpu::FeatureName::AdapterPropertiesD3D` allows querying D3D information from the adapter.

`wgpu::AdapterPropertiesD3D` may be chained on `wgpu::AdapterInfo` in a call to `wgpu::Adapter::GetInfo` or `wgpu::Device::GetAdapterInfo` in order to query D3D information on the adapter.

Adds `wgpu::AdapterPropertiesD3D` which is a struct describing the D3D adapter.
```
struct AdapterPropertiesD3D {
    uint32_t shaderModel;  // The D3D shader model
};
```

## Vulkan

`wgpu::FeatureName::AdapterPropertiesVk` allows querying Vulkan information from the adapter.

`wgpu::AdapterPropertiesVk` may be chained on `wgpu::AdapterInfo` in a call to `wgpu::Adapter::GetInfo` or `wgpu::Device::GetAdapterInfo` in order to query Vulkan information on the adapter.

Adds `wgpu::AdapterPropertiesVk` which is a struct describing the Vulkan adapter.
```
struct AdapterPropertiesVk {
    uint32_t driverVersion;  // The Vulkan driver version
};
```
