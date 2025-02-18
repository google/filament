# Timestamp Query Inside Passes (experimental!)

Adds support for the `writeTimestamp` call inside render and compute passes.
This method might not always make a lot of sense on tiler GPUs (so basically any modern GPU) where the ordering of operations inside render passes is not strictly like the order of commands recorded (each tile being executed independently),

Adds the following methods:

```
voi wgpu::ComputePassEncoder::writeTimestamp(wgpu::QuerySet querySet, uint32_t queryIndex);
voi wgpu::RenderPassEncoder::writeTimestamp(wgpu::QuerySet querySet, uint32_t queryIndex);
```

The validation is the following:

 - `querySet` must be a valid query set.
 - `querySet` must have been created with `wgpu::QueryType::Timestamp`.
 - `queryIndex` must be less than `querySet`'s size.
 - During the `wgpu::Queue::Submit` for these commands, `querySet` must not be destroyed.

The initial tracking bug was https://crbug.com/dawn/434.
