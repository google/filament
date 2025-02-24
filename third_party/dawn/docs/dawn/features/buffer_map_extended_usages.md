# Buffer Map Extended Usages

## Overview:
 - The `wgpu::Feature::BufferMapExtendedUsages` feature allows creating a buffer with `wgpu::BufferUsage::MapRead` and/or `wgpu::BufferUsage::MapWrite` and any other `wgpu::BufferUsage`.

### Example Usage:
```
wgpu::BufferDescriptor descriptor;
descriptor.size = size;
descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::Uniform;
wgpu::Buffer uniformBuffer = device.CreateBuffer(&descriptor);

uniformBuffer.MapAsync(wgpu::MapMode::Write, 0, size,
   [](WGPUBufferMapAsyncStatus status, void* userdata)
   {
      wgpu::Buffer* buffer = static_cast<wgpu::Buffer*>(userdata);
      memcpy(buffer->GetMappedRange(), data, sizeof(data));
   },
   &uniformBuffer);
```

