Basis Universal transcoder — vendored for KTX2 validation.

Source: https://github.com/BinomialLLC/basis_universal
License: Apache-2.0 (see NOTICE)

Vendored files (22 files) from transcoder/:
  basisu_transcoder.cpp — main implementation
  basisu_transcoder.h + internal headers + .inc/.inl table files

KTX2 Zstd supercompression is DISABLED (BASISD_SUPPORT_KTX2_ZSTD=0).
Set to 1 and add deps/zstd to the include path if UASTC Zstd KTX2
decoding is needed later.
