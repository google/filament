TinyEXR-private Zstandard subset
================================

`tinyexr_zstd.c` is generated from the upstream zstd source tree at
`/home/syoyo/work/zstd` using `build/single_file_libs/combine.py`, with only
the modules needed for one-shot compression and decompression.

TinyEXR selects the BSD license option offered by upstream zstd. The copied
license text is in `LICENSE`; GPL is not selected.

The generated source namespaces upstream global symbols with `tinyexr_zstd_`
to avoid collisions when an application links TinyEXR and libzstd together.
