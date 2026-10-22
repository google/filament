/*
 * TinyEXR - a small, fast, portable OpenEXR image reader/writer.
 *
 * This is the pure-C11 public API. It has no C++ in the core and links with a
 * C compiler, but the header is safe to include from C++ as well.
 *
 * Two layers are provided:
 *   - High level:  exr_load_from_file/memory(), exr_save_to_file/memory().
 *   - Mid level:   exr_reader_* / exr_writer_* for partial reads, multipart,
 *                  deep images, and optional streaming (suspend/resume) I/O.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TINYEXR_EXR_H_
#define TINYEXR_EXR_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Version
 * ========================================================================== */

#define EXR_VERSION_MAJOR 3
#define EXR_VERSION_MINOR 0
#define EXR_VERSION_PATCH 0

/* ============================================================================
 * Result codes
 * ========================================================================== */

typedef enum exr_result {
    EXR_SUCCESS = 0,
    EXR_WOULD_BLOCK = 1, /* streaming source: fetch bytes then resume */

    EXR_ERROR_INVALID_ARGUMENT = -1,
    EXR_ERROR_INVALID_FILE = -2,    /* bad magic / version / structure */
    EXR_ERROR_UNSUPPORTED = -3,     /* feature/compression not implemented */
    EXR_ERROR_OUT_OF_MEMORY = -4,
    EXR_ERROR_IO = -5,              /* file open/read/write failure */
    EXR_ERROR_CORRUPT = -6,         /* bounds/overflow/decode failure */
} exr_result;

#define EXR_OK(r) ((int)(r) >= 0)

/* Human-readable description of a result code (static string). */
const char *exr_result_string(exr_result r);

/* ============================================================================
 * Allocator hook (optional; pass NULL to use malloc/free)
 * ========================================================================== */

typedef struct exr_allocator {
    void *user;
    void *(*alloc)(void *user, size_t size);
    void (*free)(void *user, void *ptr);
} exr_allocator;

/* ============================================================================
 * Enumerations mirroring the OpenEXR format
 * ========================================================================== */

typedef enum exr_pixel_type {
    EXR_PIXEL_UINT = 0,
    EXR_PIXEL_HALF = 1,
    EXR_PIXEL_FLOAT = 2
} exr_pixel_type;

typedef enum exr_compression {
    EXR_COMPRESSION_NONE = 0,
    EXR_COMPRESSION_RLE = 1,
    EXR_COMPRESSION_ZIPS = 2,  /* zlib, 1 scanline/block */
    EXR_COMPRESSION_ZIP = 3,   /* zlib, 16 scanlines/block */
    EXR_COMPRESSION_PIZ = 4,   /* wavelet + Huffman, 32 scanlines/block */
    EXR_COMPRESSION_PXR24 = 5, /* lossy 24-bit float, 16 scanlines/block */
    EXR_COMPRESSION_B44 = 6,   /* lossy 4x4, 32 scanlines/block */
    EXR_COMPRESSION_B44A = 7,  /* B44 with flat-block packing, 32/block */
    EXR_COMPRESSION_DWAA = 8,  /* not yet supported */
    EXR_COMPRESSION_DWAB = 9,  /* not yet supported */
    EXR_COMPRESSION_HTJ2K256 = 10, /* HTJ2K/JPH, 256 scanlines/block */
    EXR_COMPRESSION_HTJ2K32 = 11,  /* HTJ2K/JPH, 32 scanlines/block */
    EXR_COMPRESSION_ZSTD = 12  /* zstd, 32 scanlines/block */
} exr_compression;

typedef enum exr_line_order {
    EXR_LINEORDER_INCREASING_Y = 0,
    EXR_LINEORDER_DECREASING_Y = 1,
    EXR_LINEORDER_RANDOM_Y = 2
} exr_line_order;

typedef enum exr_part_type {
    EXR_PART_SCANLINE = 0,
    EXR_PART_TILED = 1,
    EXR_PART_DEEP_SCANLINE = 2,
    EXR_PART_DEEP_TILED = 3
} exr_part_type;

typedef enum exr_tile_level_mode {
    EXR_TILE_ONE_LEVEL = 0,
    EXR_TILE_MIPMAP_LEVELS = 1,
    EXR_TILE_RIPMAP_LEVELS = 2
} exr_tile_level_mode;

typedef enum exr_tile_rounding_mode {
    EXR_TILE_ROUND_DOWN = 0,
    EXR_TILE_ROUND_UP = 1
} exr_tile_rounding_mode;

/* ============================================================================
 * Basic geometry
 * ========================================================================== */

typedef struct exr_box2i {
    int32_t min_x, min_y, max_x, max_y;
} exr_box2i;

/* ============================================================================
 * Channel / header / part / image (plain data; caller may read fields)
 * ========================================================================== */

#define EXR_MAX_NAME 256

typedef struct exr_channel {
    char name[EXR_MAX_NAME];
    exr_pixel_type pixel_type;
    int32_t x_sampling;
    int32_t y_sampling;
    uint8_t p_linear; /* perceptually linear hint */
} exr_channel;

/* Opaque, parsed attribute list (custom/standard attributes, for round-trip). */
typedef struct exr_attr_list exr_attr_list;

typedef struct exr_header {
    exr_part_type part_type;
    exr_compression compression;
    exr_line_order line_order;

    exr_box2i data_window;
    exr_box2i display_window;
    float pixel_aspect_ratio;
    float screen_window_center_x;
    float screen_window_center_y;
    float screen_window_width;

    /* CIE xy primaries + whitepoint, in OpenEXR order:
     * red.x,red.y, green.x,green.y, blue.x,blue.y, white.x,white.y.
     * Only valid when has_chromaticities != 0; otherwise consumers default to
     * Rec.709/sRGB primaries (the OpenEXR default). */
    uint8_t has_chromaticities;
    float chromaticities[8];

    int32_t num_channels;
    exr_channel *channels; /* sorted by name; owned by the image/reader */

    /* Tiled parts only (part_type == EXR_PART_TILED / DEEP_TILED). */
    uint8_t tiled;
    uint32_t tile_x_size;
    uint32_t tile_y_size;
    exr_tile_level_mode level_mode;
    exr_tile_rounding_mode rounding_mode;

    char name[EXR_MAX_NAME]; /* multipart part name ("" if single part) */

    exr_attr_list *attrs; /* all parsed attributes (opaque); may be NULL */
} exr_header;

typedef struct exr_part {
    exr_header header;

    int32_t width;  /* data_window width  (max_x - min_x + 1) */
    int32_t height; /* data_window height (max_y - min_y + 1) */

    /*
     * Flat (non-deep) pixel storage, planar per channel. images[c] points to
     * width*height elements of header.channels[c].pixel_type, row-major within
     * the data window. NULL for deep parts.
     */
    void **images;

    /*
     * Deep storage (is_deep != 0). deep_sample_counts[y*width+x] holds the
     * sample count per pixel. deep_images[c] is a contiguous array of all
     * samples for channel c, in pixel (row-major) order, native pixel type;
     * the samples for pixel p start at offset sum(deep_sample_counts[0..p-1]).
     */
    uint8_t is_deep;
    int32_t *deep_sample_counts; /* width*height entries, or NULL */
    void **deep_images;          /* [channel] -> contiguous samples, or NULL */
    uint64_t deep_total_samples; /* sum of deep_sample_counts */
} exr_part;

typedef struct exr_image {
    int32_t num_parts;
    exr_part *parts; /* owned */
    exr_allocator alloc;
} exr_image;

/* Release everything an exr_image owns (channels, pixels, deep arrays, parts).
 * Safe to call on a zero-initialized image. */
void exr_image_free(exr_image *img);

/* Release a single part filled by the mid-level reader (exr_reader_read_part /
 * _read_scanlines / _read_tile). Pass the allocator the reader was opened with
 * (NULL for the default). Safe on a zero-initialized part. */
void exr_part_free(const exr_allocator *a, exr_part *part);

/* ============================================================================
 * Custom header attributes (exr_header.attrs is otherwise opaque)
 *
 * Read access lets callers inspect any parsed attribute (standard or custom);
 * write access lets callers attach custom attributes that round-trip through
 * exr_save_* / exr_writer_*. The standard attributes the writer emits itself
 * (channels, compression, dataWindow, displayWindow, lineOrder,
 * pixelAspectRatio, screenWindowCenter, screenWindowWidth, tiles, name, type,
 * chunkCount, version, maxSamplesPerPixel) are skipped on write to avoid
 * duplicates.
 * ========================================================================== */

/* A read-only view onto one parsed attribute. `data` points into storage owned
 * by the exr_image/reader and is valid for its lifetime. It is the raw on-disk
 * payload and is NOT guaranteed NUL-terminated (OpenEXR "string" attributes are
 * strlen bytes with no NUL) — use exr_header_get_string_attribute for strings. */
typedef struct exr_attribute {
    const char *name;      /* attribute name (NUL-terminated) */
    const char *type_name; /* OpenEXR type, e.g. "string", "int", "float" */
    const void *data;      /* raw payload, `size` bytes */
    uint32_t size;
} exr_attribute;

/* Number of parsed attributes on a header (0 if header->attrs is NULL). */
int32_t exr_header_num_attributes(const exr_header *hdr);

/* Fill *out with attribute `index` (0..num-1). EXR_ERROR_INVALID_ARGUMENT if
 * out is NULL or index is out of range. */
exr_result exr_header_get_attribute(const exr_header *hdr, int32_t index,
                                    exr_attribute *out);

/* Find an attribute by name. EXR_SUCCESS and fills *out, or
 * EXR_ERROR_INVALID_ARGUMENT if not found / bad args. */
exr_result exr_header_find_attribute(const exr_header *hdr, const char *name,
                                     exr_attribute *out);

/* Copy a "string"-typed attribute's value into `buf` (always NUL-terminated
 * when buf_size > 0; truncated to fit). EXR_SUCCESS if the attribute exists
 * (regardless of truncation), else EXR_ERROR_INVALID_ARGUMENT. out_len
 * (optional) receives the full untruncated length. */
exr_result exr_header_get_string_attribute(const exr_header *hdr,
                                           const char *name, char *buf,
                                           size_t buf_size, size_t *out_len);

/* Set (or replace) a custom attribute so it round-trips through save/write.
 * `type_name` is the OpenEXR type string (e.g. "string", "int"); data/size is
 * the raw payload (for "string", strlen bytes, NO trailing NUL). Allocates
 * header->attrs on first use with `alloc` (NULL = default). */
exr_result exr_header_set_attribute(const exr_allocator *alloc, exr_header *hdr,
                                    const char *name, const char *type_name,
                                    const void *data, uint32_t size);

/* Convenience for "string"-typed attributes (writes strlen(value) bytes). */
exr_result exr_header_set_string_attribute(const exr_allocator *alloc,
                                           exr_header *hdr, const char *name,
                                           const char *value);

/* ============================================================================
 * High-level load / save
 * ========================================================================== */

/* Load an entire EXR (all parts, all channels, native pixel types). On success
 * *out is filled and must be released with exr_image_free(). */
exr_result exr_load_from_file(const char *path, const exr_allocator *alloc,
                              exr_image *out);
exr_result exr_load_from_memory(const void *data, size_t size,
                                const exr_allocator *alloc, exr_image *out);

/* Save an image. The image's parts/channels/pixels must be populated by the
 * caller (typically built by hand or returned from a loader). */
exr_result exr_save_to_file(const char *path, const exr_image *img,
                            exr_compression compression);
exr_result exr_save_to_memory(void **out_data, size_t *out_size,
                              const exr_allocator *alloc, const exr_image *img,
                              exr_compression compression);

/* ============================================================================
 * Mid-level reader
 * ========================================================================== */

typedef struct exr_reader exr_reader;

/*
 * Streaming data source. Only used by exr_reader_open_source(). The callback
 * must copy [off, off+len) of the file into dst and return EXR_SUCCESS, or
 * return EXR_WOULD_BLOCK if those bytes are not yet available (the host then
 * fetches them, calls exr_reader_supply(), and re-invokes the reader call).
 */
typedef struct exr_data_source {
    void *user;
    exr_result (*read)(void *user, uint64_t off, uint64_t len, void *dst);
    uint64_t total_size; /* 0 if unknown */
} exr_data_source;

/* The memory path is zero-copy and never returns EXR_WOULD_BLOCK. The data
 * buffer must stay valid for the lifetime of the reader. */
exr_result exr_reader_open_memory(const void *data, size_t size,
                                  const exr_allocator *alloc, exr_reader **out);
exr_result exr_reader_open_source(const exr_data_source *src,
                                  const exr_allocator *alloc, exr_reader **out);
/* Convenience: open a reader on a file path (stdio). Defined in the optional
 * exr_stdio.c module; unavailable in freestanding builds that omit it. */
exr_result exr_reader_open_file(const char *path, const exr_allocator *alloc,
                                exr_reader **out);
void exr_reader_close(exr_reader *r);

/* Parse the file/version + all part headers + offset tables (no pixel I/O).
 * May return EXR_WOULD_BLOCK in streaming mode. */
exr_result exr_reader_parse_header(exr_reader *r);

int32_t exr_reader_num_parts(const exr_reader *r);
const exr_header *exr_reader_part_header(const exr_reader *r, int32_t part);

/*
 * Read pixels. Each call appends ownership into *out (a caller-provided,
 * zero-initialized exr_part for the single-part calls). May return
 * EXR_WOULD_BLOCK in streaming mode.
 */
exr_result exr_reader_read_part(exr_reader *r, int32_t part, exr_part *out);
exr_result exr_reader_read_scanlines(exr_reader *r, int32_t part,
                                     int32_t y_start, int32_t y_count,
                                     exr_part *out);
exr_result exr_reader_read_tile(exr_reader *r, int32_t part, int32_t tile_x,
                                int32_t tile_y, int32_t level_x, int32_t level_y,
                                exr_part *out);

/* ---- Streaming suspend / resume (only meaningful with a data source) ---- */
typedef struct exr_pending_read {
    uint64_t offset;
    uint64_t size;
} exr_pending_read;

/* When a reader call returns EXR_WOULD_BLOCK, query the bytes it needs. */
exr_result exr_reader_pending(const exr_reader *r, exr_pending_read *out);
/* Hand the fetched bytes back, then re-call the same reader function. */
exr_result exr_reader_supply(exr_reader *r, const void *data, size_t size);

/* ============================================================================
 * Mid-level writer
 * ========================================================================== */

typedef struct exr_writer exr_writer;

exr_result exr_writer_create(const exr_allocator *alloc, exr_writer **out);
void exr_writer_destroy(exr_writer *w);

/* Add a part described by hdr (its channel list + windows + compression).
 * Returns the new part index in *out_part (may be NULL). */
exr_result exr_writer_add_part(exr_writer *w, const exr_header *hdr,
                               int32_t *out_part);

/* Provide planar pixel data for one channel of a part (width*height elements
 * of the channel's pixel type). Pointer must remain valid until finalize. */
exr_result exr_writer_set_channel(exr_writer *w, int32_t part, const char *name,
                                  const void *pixels);

exr_result exr_writer_finalize_to_memory(exr_writer *w, void **out_data,
                                         size_t *out_size);
exr_result exr_writer_finalize_to_file(exr_writer *w, const char *path);

/* ============================================================================
 * Streaming block I/O (bounded working memory)
 *
 * The calls above materialize a whole part at once. The block API lets a caller
 * process exactly one scanline-block or one tile at a time, so peak memory is a
 * single block rather than the entire image. A "block" is one offset-table
 * chunk: one scanline block for scanline parts, one tile (at one level) for
 * tiled parts; mipmap/ripmap levels and deep parts are covered too.
 * ========================================================================== */

/* Geometry of one block (chunk). Filled by exr_reader_block_info(). */
typedef struct exr_block_info {
    int32_t part;
    uint8_t is_tiled;          /* tiled or deep-tiled part */
    uint8_t is_deep;
    int32_t y0;                /* block first scanline (data-window y) */
    int32_t tile_x, tile_y;    /* tiled: tile indices within the level */
    int32_t level_x, level_y;  /* tiled: level (0,0 for ONE_LEVEL/scanline) */
    int32_t x0;                /* block origin x (absolute data-window coords) */
    int32_t width, height;     /* block pixel extent (height == #scanlines) */
    size_t  uncompressed_size; /* flat parts: bytes for the canonical buffer */
} exr_block_info;

/* ---- streaming decode ---- */

/* Number of chunks (offset-table entries) in a part, across all levels. */
exr_result exr_reader_num_blocks(exr_reader *r, int32_t part, uint32_t *out);

/* Geometry + buffer size for block `idx`. Derived from the offset-table / level
 * math; performs no pixel I/O and never returns EXR_WOULD_BLOCK. */
exr_result exr_reader_block_info(exr_reader *r, int32_t part, uint32_t idx,
                                 exr_block_info *out);

/* Flat parts: decode block `idx` into `dst` (>= info.uncompressed_size) in the
 * canonical layout (per scanline, then per channel in name-sorted order, sample
 * data only). May return EXR_WOULD_BLOCK in streaming mode (re-call after
 * supply). Use exr_block_extract_channel() to unpack a single channel. */
exr_result exr_reader_decode_block(exr_reader *r, int32_t part, uint32_t idx,
                                   void *dst, size_t dst_size);

/* Copy one channel out of a decoded canonical flat block into a tight planar
 * buffer: exr_num_samples(x0,x0+w-1,xs) * exr_num_samples(y0,y0+h-1,ys) elements
 * of that channel's pixel type, row-major. `channel` indexes the name-sorted
 * channel order (== header->channels order). */
exr_result exr_block_extract_channel(const exr_header *h,
                                     const exr_block_info *info,
                                     const void *block, size_t block_size,
                                     int32_t channel, void *dst);

/* Deep parts: counts must be known before sample buffers can be sized, so decode
 * is two-step. `counts` holds info.width*info.height per-pixel counts (block
 * row-major). After summing them, size chan_dst[c] to sum(counts) elements of
 * channel c's pixel type and call _decode_deep_samples. Both may WOULD_BLOCK. */
exr_result exr_reader_decode_deep_counts(exr_reader *r, int32_t part,
                                         uint32_t idx, int32_t *counts);
exr_result exr_reader_decode_deep_samples(exr_reader *r, int32_t part,
                                          uint32_t idx, void *const *chan_dst);

/* ---- streaming encode ---- */

/* Seekable output sink. write() appends `len` bytes; seek() repositions for the
 * offset-table backpatch at end_stream. close() is optional (may be NULL) and is
 * invoked exactly once when the stream finishes or the writer is destroyed — it
 * lets a sink release its resource (e.g. fclose) without the core touching libc.
 * write/seek/close return EXR_SUCCESS or an error. */
typedef struct exr_data_sink {
    void *user;
    exr_result (*write)(void *user, const void *data, size_t len);
    exr_result (*seek)(void *user, uint64_t off);
    exr_result (*close)(void *user); /* optional; may be NULL */
} exr_data_sink;

/* Begin a streaming encode. Parts must already be described via
 * exr_writer_add_part (channels, windows, tiling); do NOT call
 * exr_writer_set_channel. Writes magic+version+headers+zeroed offset tables to
 * the sink immediately, then expects one write call per block. `comp` overrides
 * every part's compression (use the part header's value by passing it). After
 * begin_stream the writer is in streaming mode until end_stream. */
exr_result exr_writer_begin_stream(exr_writer *w, const exr_data_sink *sink,
                                   exr_compression comp);
/* Convenience: stream straight to a file (fwrite + fseek backpatch). */
exr_result exr_writer_begin_stream_file(exr_writer *w, const char *path,
                                        exr_compression comp);

/* Feed one flat scanline block. y0 must be a block boundary. channel_rows[c]
 * points to this block's planar samples for channel c (header->channels order):
 * exr_num_samples(xmin,xmax,xs) * exr_num_samples(y0,y0+nlines-1,ys) elements. */
exr_result exr_writer_write_scanline_block(exr_writer *w, int32_t part,
                                           int32_t y0,
                                           const void *const *channel_rows);

/* Feed one flat tile at level (level_x,level_y) (0,0 for ONE_LEVEL). For
 * mipmap/ripmap the caller supplies every (tile,level) itself (no pyramid
 * auto-generation). channel_data[c] is the tile's planar samples for channel c. */
exr_result exr_writer_write_tile(exr_writer *w, int32_t part, int32_t tile_x,
                                 int32_t tile_y, int32_t level_x, int32_t level_y,
                                 const void *const *channel_data);

/* Deep variants. counts[] holds width*height per-pixel sample counts (block
 * row-major); chan_samp[c] holds the block's contiguous samples for channel c
 * in pixel row-major order. */
exr_result exr_writer_write_deep_scanline_block(exr_writer *w, int32_t part,
                                                int32_t y0, const int32_t *counts,
                                                const void *const *chan_samp);
exr_result exr_writer_write_deep_tile(exr_writer *w, int32_t part,
                                      int32_t tile_x, int32_t tile_y,
                                      int32_t level_x, int32_t level_y,
                                      const int32_t *counts,
                                      const void *const *chan_samp);

/* Backpatch every offset table (seek + write) and flush. For the file-backed
 * sink this also closes the file. */
exr_result exr_writer_end_stream(exr_writer *w);

/* ============================================================================
 * Spectral images (JCGT 2021 "An OpenEXR Layout for Spectral Images";
 * afichet/spectral-exr)
 *
 * Spectral data is stored as ordinary EXR channels named by wavelength, with a
 * comma decimal separator and an "nm" suffix:
 *   - emissive Stokes component n (0..3): "S{n}.{wavelength}nm" (e.g.
 *     "S0.550,000000nm"); S0 is intensity, S1..S3 the polarisation state.
 *   - reflective:                         "T.{wavelength}nm".
 * A "spectralLayoutVersion" string attribute (value "1.0") marks the file as
 * spectral; units live in "ROOT/units" (reflective) or "emissiveUnits"
 * (emissive/polarised), with optional "polarisationHandedness".
 * ========================================================================== */

typedef enum exr_spectrum_type {
    EXR_SPECTRUM_NONE = 0,   /* not a spectral image */
    EXR_SPECTRUM_REFLECTIVE, /* T.{wl}nm channels */
    EXR_SPECTRUM_EMISSIVE,   /* S0.{wl}nm only */
    EXR_SPECTRUM_POLARISED   /* S0..S3.{wl}nm */
} exr_spectrum_type;

#define EXR_SPECTRAL_LAYOUT_VERSION "1.0"

/* True if the header carries a "spectralLayoutVersion" attribute. */
int exr_is_spectral(const exr_header *hdr);

/* Classify by channel naming (REFLECTIVE / EMISSIVE / POLARISED), else NONE. */
exr_spectrum_type exr_spectrum_type_of(const exr_header *hdr);

/* Parse the wavelength (nm) from a spectral channel name; < 0 if not spectral.
 * Locale-independent; accepts the comma decimal separator. */
float exr_spectral_channel_wavelength(const char *channel_name);

/* Stokes component 0..3 for "S{n}." channels, or -1 (incl. all "T." channels). */
int exr_spectral_channel_stokes(const char *channel_name);

/* 1 if the name matches S{n}.{wl}nm or T.{wl}nm with a valid wavelength. */
int exr_is_spectral_channel(const char *channel_name);

/* Collect the sorted, unique wavelengths (ascending) into `out` (capacity
 * `max`). Returns the count written; if out is NULL returns the count that
 * would be written (query). Wavelengths within 0.01 nm are merged. NOTE: the
 * header channel order is lexicographic on the formatted wavelength string, so
 * it does NOT equal ascending wavelength — always rely on this sort. */
int32_t exr_spectral_wavelengths(const exr_header *hdr, float *out, int32_t max);

/* Copy the spectral units string ("ROOT/units" for reflective, else
 * "emissiveUnits") into buf (NUL-terminated). EXR_SUCCESS if present, else
 * EXR_ERROR_INVALID_ARGUMENT. */
exr_result exr_spectral_units(const exr_header *hdr, char *buf, size_t buf_size);

/* Build a channel name into buf (>= 32 bytes). EMISSIVE/POLARISED use stokes
 * 0..3; REFLECTIVE ignores stokes. Format "S{n}.{wl}nm" / "T.{wl}nm" with a
 * comma decimal separator and 6 fractional digits. */
void exr_spectral_channel_name(char *buf, size_t buf_size,
                               exr_spectrum_type type, int stokes,
                               float wavelength_nm);

/* Attach spectralLayoutVersion + units (+ polarisationHandedness for POLARISED)
 * to a header so it round-trips through save. `units` may be NULL. */
exr_result exr_spectral_set_attributes(const exr_allocator *alloc,
                                       exr_header *hdr, exr_spectrum_type type,
                                       const char *units);

/* A decoded spectral cube (part 0 only; all planes float). */
typedef struct exr_spectral_image {
    int32_t width, height;
    exr_spectrum_type type;

    int32_t num_wavelengths;
    float *wavelengths; /* [num_wavelengths] ascending, owned */

    /* Stokes planes. stokes[0] is always present (S0 emissive, or T reflective);
     * stokes[1..3] hold S1..S3 for POLARISED, else NULL. Each non-NULL plane is
     * num_wavelengths * width * height floats, laid out [wavelength][y*width+x]
     * (num_wavelengths contiguous width*height planes). */
    float *stokes[4];

    char units[64];      /* "" if none */
    char handedness[8];  /* "left"/"right"/"" (polarised only) */
    exr_allocator alloc; /* owns the buffers above */
} exr_spectral_image;

/* Load + verify spectral + gather wavelengths + fill the float cube. On success
 * *out is filled and must be released with exr_spectral_image_free().
 * EXR_ERROR_UNSUPPORTED if the file is not spectral (or uses subsampled
 * spectral channels, or is multipart-only-spectral past part 0). */
exr_result exr_spectral_load_from_file(const char *path,
                                       const exr_allocator *alloc,
                                       exr_spectral_image *out);
exr_result exr_spectral_load_from_memory(const void *data, size_t size,
                                         const exr_allocator *alloc,
                                         exr_spectral_image *out);

void exr_spectral_image_free(exr_spectral_image *img);

/* Sample at (wavelength_index, x, y) of Stokes plane `s` (0 for non-polarised).
 * Returns 0 on out-of-range. */
float exr_spectral_sample(const exr_spectral_image *img, int32_t s,
                          int32_t wavelength_index, int32_t x, int32_t y);

/* Copy the full per-pixel spectrum of plane `s` at (x,y) into out_spectrum
 * (>= num_wavelengths floats). Returns the count written, 0 on bad args. */
int32_t exr_spectral_pixel(const exr_spectral_image *img, int32_t s, int32_t x,
                           int32_t y, float *out_spectrum);

const float *exr_spectral_wavelength_array(const exr_spectral_image *img,
                                           int32_t *out_count);

/* Build a single-part exr_image (channels named per the convention, FLOAT
 * pixels) plus the spectral attributes, ready for exr_save_to_file/memory.
 * `samples` is [wavelength][y*width+x]. `units` may be NULL. On success *out_img
 * is filled and owned by the caller (free with exr_image_free). */
exr_result exr_spectral_setup_emissive(const exr_allocator *alloc, int32_t width,
                                       int32_t height, int32_t num_wavelengths,
                                       const float *wavelengths,
                                       const float *samples, const char *units,
                                       exr_image *out_img);
exr_result exr_spectral_setup_reflective(const exr_allocator *alloc,
                                         int32_t width, int32_t height,
                                         int32_t num_wavelengths,
                                         const float *wavelengths,
                                         const float *samples, const char *units,
                                         exr_image *out_img);

/* ============================================================================
 * Utilities
 * ========================================================================== */

/* Identify an EXR by magic number. */
int exr_is_exr_memory(const void *data, size_t size);

/* Runtime SIMD capability bits (for diagnostics). */
typedef enum exr_simd_caps {
    EXR_SIMD_NONE = 0,
    EXR_SIMD_SSE2 = 1u << 0,
    EXR_SIMD_SSE41 = 1u << 1,
    EXR_SIMD_AVX2 = 1u << 2,
    EXR_SIMD_NEON = 1u << 3
} exr_simd_caps;

uint32_t exr_simd_capabilities(void);
const char *exr_simd_info(void);

/* zlib (DEFLATE) backend used by ZIP/ZIPS/PXR24. The in-tree pure-C codec is
 * always available; the vendored libdeflate is compiled in for hosted builds
 * (DEFLATE=auto|libdeflate) and is faster on natural-image data. Freestanding
 * and WASM builds only ever have the in-tree codec. */
typedef enum exr_zlib_backend {
    EXR_ZLIB_AUTO = 0,    /* the build-time default backend */
    EXR_ZLIB_INTREE,      /* force the in-tree pure-C codec */
    EXR_ZLIB_LIBDEFLATE   /* force libdeflate (else EXR_ERROR_UNSUPPORTED) */
} exr_zlib_backend;

/* Select the zlib backend for subsequent (de)compression. Not thread-safe with
 * concurrent decoding: call before kicking off a decode, not during one.
 * Returns EXR_ERROR_UNSUPPORTED if libdeflate is requested but not compiled in
 * (the backend is left unchanged). */
exr_result exr_zlib_set_backend(exr_zlib_backend backend);

/* Name of the active zlib backend ("in-tree" or "libdeflate"). */
const char *exr_zlib_backend_name(void);

/* Pixel-format conversion (runtime SIMD-dispatched). */
void exr_half_to_float(const uint16_t *src, float *dst, size_t count);
void exr_float_to_half(const float *src, uint16_t *dst, size_t count);

/* ============================================================================
 * Image utilities (post-decode processing on float/half/uint pixel data)
 *
 * These operate on plain typed buffers, decoupled from the EXR containers; thin
 * exr_part bridges (further down) gather/scatter the planar EXR storage. The
 * canonical working format is interleaved float32 RGB(A); half/uint inputs are
 * widened at the boundary and narrowed on output. All entry points are pure C
 * and libm-free (the few transcendentals used by transfer functions are
 * hand-rolled), so the whole module is available in freestanding builds.
 *
 * Hot paths are runtime SIMD-dispatched (SSE2/AVX2/F16C, NEON) with a scalar
 * reference that is the source of truth.
 * ========================================================================== */

/* ---- Pixel-format conversion helpers ------------------------------------- */

/* RAW: numeric cast (uint 5 -> 5.0f). NORMALIZED: full integer range <-> [0,1]
 * (u16/65535, u32/4294967295). For float->uint the value is clamped then
 * rounded to nearest (ties to even). Note: RAW uint32 values > 2^24 are not
 * exactly representable in float32 and are rounded. */
typedef enum exr_convert_mode {
    EXR_CONVERT_RAW = 0,
    EXR_CONVERT_NORMALIZED = 1
} exr_convert_mode;

/* Convert `count` elements between any two EXR pixel types (HALF/FLOAT/UINT). */
exr_result exr_convert_pixels(void *dst, exr_pixel_type dst_type, const void *src,
                              exr_pixel_type src_type, size_t count,
                              exr_convert_mode mode);

/* 8-/16-bit integer <-> float interop (PNG/screenshot pipelines). `normalized`
 * selects [0,1] scaling vs a raw numeric cast. float->int clamps + rounds. */
void exr_u8_to_float(const uint8_t *src, float *dst, size_t n, int normalized);
void exr_u16_to_float(const uint16_t *src, float *dst, size_t n, int normalized);
void exr_float_to_u8(const float *src, uint8_t *dst, size_t n, int normalized);
void exr_float_to_u16(const float *src, uint16_t *dst, size_t n, int normalized);

/* ---- HDR resize / resampling --------------------------------------------- */

typedef enum exr_resize_filter {
    EXR_RESIZE_BOX = 0,         /* nearest-area average */
    EXR_RESIZE_TRIANGLE = 1,    /* linear / bilinear */
    EXR_RESIZE_CATMULL_ROM = 2, /* sharp interpolating cubic */
    EXR_RESIZE_MITCHELL = 3     /* smooth cubic (B=C=1/3); good default */
} exr_resize_filter;

typedef enum exr_edge_mode {
    EXR_EDGE_CLAMP = 0,
    EXR_EDGE_REFLECT = 1,
    EXR_EDGE_WRAP = 2
} exr_edge_mode;

/* Resize interleaved float32 in linear light (HDR-safe: values are never
 * clamped). `channels` is 1..4; row strides are in elements (0 = tight, i.e.
 * width*channels). If `alpha_channel` >= 0 the RGB channels are resampled
 * premultiplied by that alpha and un-premultiplied afterwards. */
exr_result exr_resize_float(const exr_allocator *a, const float *src, int src_w,
                            int src_h, size_t src_row_stride, float *dst,
                            int dst_w, int dst_h, size_t dst_row_stride,
                            int channels, exr_resize_filter filter,
                            exr_edge_mode edge, int alpha_channel);

/* Streaming resizer: O(filter_support * dst_w) memory, independent of image
 * height. Push source rows (top to bottom) and pull destination rows; pull
 * returns EXR_WOULD_BLOCK when more source rows are required first. `io_type`
 * is the pixel type of the rows passed to push/pull (HALF/FLOAT/UINT). */
typedef struct exr_resizer exr_resizer;

exr_result exr_resizer_create(const exr_allocator *a, int src_w, int src_h,
                              int dst_w, int dst_h, int channels,
                              exr_pixel_type io_type, exr_resize_filter filter,
                              exr_edge_mode edge, exr_resizer **out);
/* Feed source scanline `src_y` (must be supplied in increasing order). */
exr_result exr_resizer_push_row(exr_resizer *r, int src_y, const void *src_row);
/* Emit the next destination scanline into `dst_row`; *out_dst_y receives its
 * index. Returns EXR_WOULD_BLOCK if more source rows must be pushed first, or
 * EXR_SUCCESS with *out_dst_y == dst_h once finished. */
exr_result exr_resizer_pull_row(exr_resizer *r, int *out_dst_y, void *dst_row);
void exr_resizer_destroy(exr_resizer *r);

/* ---- Tonemapping (linear-light float RGB -> display-referred) ------------- */

typedef enum exr_tonemap_op {
    EXR_TONEMAP_REINHARD = 0,
    EXR_TONEMAP_REINHARD_EXT = 1, /* with white point */
    EXR_TONEMAP_ACES = 2,         /* Narkowicz fitted RRT+ODT */
    EXR_TONEMAP_HABLE = 3         /* Uncharted2 filmic */
} exr_tonemap_op;

typedef struct exr_tonemap_params {
    float exposure;    /* linear pre-scale; 0 is treated as 1.0 */
    float white_point; /* REINHARD_EXT (Lwhite); 0 is treated as 1.0 */
    float A, B, C, D, E, F, W; /* HABLE curve; all-zero uses Uncharted2 defaults */
} exr_tonemap_params;

/* Operates on channels 0..min(channels,3)-1; any 4th (alpha) passes through.
 * In-place (dst == src) is allowed. `params` may be NULL for defaults. */
exr_result exr_tonemap_float(float *dst, const float *src, size_t pixel_count,
                             int channels, exr_tonemap_op op,
                             const exr_tonemap_params *params);

/* ---- Colorspace conversion ----------------------------------------------- */

typedef enum exr_colorspace {
    EXR_CS_SRGB = 0, /* Rec.709 primaries, D65 (alias for Rec.709 linear) */
    EXR_CS_REC709 = 0,
    EXR_CS_REC2020 = 1,
    EXR_CS_ACES_AP0 = 2, /* ACES2065-1, ~D60 */
    EXR_CS_ACES_AP1 = 3, /* ACEScg, ~D60 */
    EXR_CS_XYZ = 4
} exr_colorspace;

/* Fill `m` (row-major 3x3) with the linear RGB->RGB matrix taking `from`
 * primaries to `to` primaries (Bradford-adapted across whitepoints). */
exr_result exr_color_matrix(exr_colorspace from, exr_colorspace to, float m[9]);

/* Fill `yw` with the RGB luminance weights (summing to 1) derived from CIE
 * chromaticities (OpenEXR order: red/green/blue/white xy). When `present` is 0
 * or the primaries are degenerate, falls back to Rec.709 (0.2126, 0.7152,
 * 0.0722). Pass `&header.chromaticities[0]` and `header.has_chromaticities`. */
exr_result exr_luminance_weights(const float chroma[8], int present,
                                 float yw[3]);

/* Apply a row-major 3x3 to interleaved float RGB(A); alpha passes through.
 * In-place allowed. */
exr_result exr_color_apply_matrix(float *dst, const float *src,
                                  size_t pixel_count, int channels,
                                  const float m[9]);

/* Transfer functions (per element; operate on each value independently).
 * encode = OETF (linear -> code), decode = EOTF (code -> linear). */
typedef enum exr_transfer {
    EXR_TF_LINEAR = 0,
    EXR_TF_SRGB = 1,
    EXR_TF_GAMMA_22 = 2,
    EXR_TF_GAMMA_24 = 3,
    EXR_TF_REC709 = 4,
    EXR_TF_PQ = 5, /* SMPTE ST 2084 (1.0 == 10000 cd/m^2) */
    EXR_TF_HLG = 6 /* ARIB STD-B67 */
} exr_transfer;

exr_result exr_encode_transfer(float *dst, const float *src, size_t count,
                               exr_transfer tf);
exr_result exr_decode_transfer(float *dst, const float *src, size_t count,
                               exr_transfer tf);

/* ---- Baked 3D LUT (apply OCIO/.cube output without OCIO) ------------------ */

typedef enum exr_lut_interp {
    EXR_LUT_TRILINEAR = 0,
    EXR_LUT_TETRAHEDRAL = 1
} exr_lut_interp;

/* `data` holds size^3 RGB triples, R-fastest: sample (ir,ig,ib) is at index
 * ((ib*size + ig)*size + ir)*3. Inputs are mapped through [domain_min,
 * domain_max] then clamped to the grid. */
typedef struct exr_lut3d {
    int size;
    const float *data;
    float domain_min[3];
    float domain_max[3];
} exr_lut3d;

exr_result exr_lut3d_apply(float *dst, const float *src, size_t pixel_count,
                           int channels, const exr_lut3d *lut,
                           exr_lut_interp interp);

/* Parse a baked Adobe/OCIO ".cube" 3D LUT from a memory blob. On success *out
 * references *out_owned (the malloc'd sample array); free *out_owned with the
 * same allocator. 1D ".cube" LUTs are rejected (EXR_ERROR_UNSUPPORTED). */
exr_result exr_lut3d_parse_cube(const exr_allocator *a, const char *text,
                                size_t len, exr_lut3d *out, float **out_owned);

/* ---- exr_part <-> interleaved float bridges ------------------------------ */

/* Gather a part's channels (sorted order; subsampling expanded to full res,
 * any pixel type widened to float) into a freshly allocated interleaved buffer.
 * *out must be freed with the same allocator. */
exr_result exr_part_to_rgba_float(const exr_allocator *a, const exr_part *part,
                                  float **out, int *out_width, int *out_height,
                                  int *out_channels);

/* True (1) when `part` is a luminance-chroma image: a full-res `Y` plus
 * subsampled `RY`/`BY` chroma and no direct `R`/`G`/`B` channels. Such parts
 * carry color that exr_part_to_rgba_float leaves as raw Y/RY/BY (grayscale to a
 * naive viewer); use exr_part_yc_to_rgba_float to reconstruct true color. */
int exr_part_is_luminance_chroma(const exr_part *part);

/* Reconstruct a luminance-chroma (`Y`/`RY`/`BY`) part into a freshly allocated
 * interleaved RGBA float buffer (`w*h*4`, alpha = `A` channel or 1.0). Chroma is
 * upsampled to full resolution and combined with Y using luminance weights from
 * the header's chromaticities (Rec.709 when absent). *out must be freed with the
 * same allocator. Returns EXR_ERROR_INVALID_ARGUMENT if the part is not
 * luminance-chroma (test with exr_part_is_luminance_chroma first). */
exr_result exr_part_yc_to_rgba_float(const exr_allocator *a,
                                     const exr_part *part, float **out,
                                     int *out_width, int *out_height);

/* Scatter an interleaved float buffer into freshly allocated planar channels of
 * `dst_type`, filling `out` (a single-part-style exr_part: header.channels and
 * images are allocated and owned; release with exr_part_free). Channels are
 * named R,G,B,A for channels 1..4. */
exr_result exr_rgba_float_to_part(const exr_allocator *a, const float *rgba,
                                  int width, int height, int channels,
                                  exr_pixel_type dst_type, exr_part *out);

/* Worker-thread count for per-block parallel encode/decode. 0 or 1 means
 * single-threaded (the default). This is a no-op unless the library is built
 * with thread support (-DEXR_USE_THREADS / `make ... THREADS=1`); without it
 * encode/decode always run single-threaded regardless of this setting.
 *
 * Parallelism applies to the in-memory load/save paths for scanline and
 * single-level tiled parts; deep, mipmap/ripmap, and the streaming reader/writer
 * APIs remain single-threaded. When threads > 1, any custom exr_allocator passed
 * to load/save must be thread-safe. The setting is process-global. */
void exr_set_num_threads(int n);
int exr_get_num_threads(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TINYEXR_EXR_H_ */
