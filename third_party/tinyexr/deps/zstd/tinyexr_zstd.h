/*
 * TinyEXR private Zstandard wrapper declarations.
 *
 * The implementation is generated from upstream zstd and uses zstd's BSD
 * license option. See deps/zstd/LICENSE.
 */

#ifndef TINYEXR_ZSTD_H_
#define TINYEXR_ZSTD_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t tinyexr_zstd_compress(void *dst, size_t dst_capacity, const void *src,
                             size_t src_size, int compression_level);
size_t tinyexr_zstd_decompress(void *dst, size_t dst_capacity, const void *src,
                               size_t src_size);
size_t tinyexr_zstd_compress_bound(size_t src_size);
unsigned tinyexr_zstd_is_error(size_t code);

/* No-malloc static-workspace decode (zstd's embedded API, renamed). Used by the
 * freestanding build where zstd's internal malloc is stubbed out: allocate a
 * workspace of tinyexr_zstd_ZSTD_estimateDCtxSize() bytes from a caller
 * allocator, init a static DCtx over it, and one-shot decode into dst (back-refs
 * read from dst, so the workspace is small and constant). ZSTD_DCtx is opaque. */
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
size_t tinyexr_zstd_ZSTD_estimateDCtxSize(void);
ZSTD_DCtx *tinyexr_zstd_ZSTD_initStaticDCtx(void *workspace,
                                            size_t workspace_size);
size_t tinyexr_zstd_ZSTD_decompressDCtx(ZSTD_DCtx *dctx, void *dst,
                                        size_t dst_capacity, const void *src,
                                        size_t src_size);

#ifdef __cplusplus
}
#endif

#endif /* TINYEXR_ZSTD_H_ */
