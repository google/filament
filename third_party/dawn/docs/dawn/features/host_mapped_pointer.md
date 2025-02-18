# Host Mapped Pointer (Experimental!)

## Overview

Enable this feature by requesting `wgpu::FeatureName::HostMappedPointer` on device creation.

Allows creation of buffers from host-mapped pointers. These may be pointers from shared memory allocations, memory-mapped files, etc. The created buffer does not own a reference to the underlying memory so any use of it will become undefined if the host memory is unmapped or freed.

## Requirements
 - `wgpu::FeatureName::HostMappedPointer` must be supported and enabled.
 - Both the address of the pointer and the size of the allocation that the pointer refers to must be aligned to OS-specific requirements. This is 4Kb on Mac / Linux and 64Kb on Windows.
 - None of the mapping APIs may actually be called on the buffer since it is effectively persistently mapped.
 - On Windows, the pointer must point to the start of the virtual or mapped memory. It will be invalid to pass a pointer returned from `MapViewOfFile` at a non-zero offset.

## Example Usage
```c++
// Memory map a file.
void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

wgpu::BufferHostMappedPointer hostMappedDesc;
hostMappedDesc.pointer = ptr;

// Use the dispose callback to be notified when the buffer is destroyed and no
// longer in use on the GPU. After this point, it is safe to unmap the memory.
hostMappedDesc.disposeCallback = [](void* userdata) {
  // Unmap and close the file.
  auto* data = reinterpret_cast<std::tuple<int, void*, size_t>*>(userdata);
  munmap(std::get<1>(*data), std::get<2>(*data));
  close(std::get<0>(*data));
  delete data;
};
// Pass the fd, ptr, and size as the userdata so we can free the
// memory later.
hostMappedDesc.userdata = new std::tuple<int, void*, size_t>(fd, ptr, size);

wgpu::BufferDescriptor bufferDesc;
bufferDesc.usage = wgpu::BufferUsage::CopySrc;
bufferDesc.size = size;
bufferDesc.nextInChain = &hostMappedDesc;

wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
```

It's expected that the memory pages from wrapped host allocations and mapped files are made resident and then pinned at the time of buffer creation. Thus, the memory should not be paged out when used on the GPU. Explicitly requesting that it be decommitted, for example using `madvise` with `MADV_DONTNEED` or `VirtualFree` with `MEM_DECOMMIT` is undefined behavior.

## TODO(crbug.com/dawn/2018):
 - Investigate adding a chained struct for importing from a Windows file HANDLE
 - Expose alignment as a proper WebGPU limit.
 - Figure out interaction with buffer mapping. These types of buffers are essentially persistently mapped.
 - Flesh out if/when the dispose callback is called when there is a validation, out-of-memory, or internal error on creation.
