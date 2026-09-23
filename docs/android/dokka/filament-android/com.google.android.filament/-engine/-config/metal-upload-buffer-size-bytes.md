//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[metalUploadBufferSizeBytes](metal-upload-buffer-size-bytes.md)

# metalUploadBufferSizeBytes

[main]\
open var [metalUploadBufferSizeBytes](metal-upload-buffer-size-bytes.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

When uploading vertex or index data, the Filament Metal backend copies data into a shared staging area before transferring it to the GPU. 

This setting controls the total size of the buffer used to perform these allocations.

Higher values can improve performance when performing many uploads across a small number of frames.

This buffer remains alive throughout the lifetime of the Engine, so this size adds to the memory footprint of the app and should be set as conservative as possible.

A value of 0 disables the shared staging buffer entirely; uploads will acquire an individual buffer from a pool of shared buffers.

Only respected by the Metal backend.
