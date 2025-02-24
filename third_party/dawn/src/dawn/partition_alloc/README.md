This directory provide Dawn a "no-op" partition_alloc implementation of raw_ptr. This is used only
when `dawn_partition_alloc_dir` is not set.

# Why not using the PartitionAlloc "no-op" implementation?
For now, we decided Dawn to provide its own. This allow the partition_alloc dependency to be
optional. It makes it easier easier to integrate into other project (Skia,
etc...) and build system (CMake, Bazel, etc..)

Moreover, partition_alloc is still depending on chromium's //build for now. It
means it can only be used inside Dawn who has the same dependency. It can't be
used outside.
