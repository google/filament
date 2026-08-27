/*
 * TinyEXR - optional stdio convenience layer.
 *
 * This is the ONLY translation unit in the library that uses <stdio.h>. The rest
 * of the core does all I/O through caller callbacks (exr_data_source /
 * exr_data_sink), so it can build freestanding. Link this TU for the convenient
 * path-based helpers; omit it for embedded/WASM targets.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

#include <stdio.h>

/* ---- whole-file load --------------------------------------------------- */

exr_result exr_load_from_file(const char *path, const exr_allocator *alloc,
                              exr_image *out) {
    FILE *fp;
    long sz;
    size_t n;
    uint8_t *buf;
    exr_result rc;

    if (!path || !out) return EXR_ERROR_INVALID_ARGUMENT;
    if (!alloc) alloc = exr_default_allocator();
    memset(out, 0, sizeof(*out));

    fp = fopen(path, "rb");
    if (!fp) return EXR_ERROR_IO;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return EXR_ERROR_IO; }
    sz = ftell(fp);
    if (sz < 0) { fclose(fp); return EXR_ERROR_IO; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return EXR_ERROR_IO; }
    buf = (uint8_t *)exr_malloc(alloc, (size_t)sz);
    if (!buf) { fclose(fp); return EXR_ERROR_OUT_OF_MEMORY; }
    n = fread(buf, 1, (size_t)sz, fp);
    fclose(fp);
    if (n != (size_t)sz) { exr_free(alloc, buf); return EXR_ERROR_IO; }

    rc = exr_load_from_memory(buf, (size_t)sz, alloc, out);
    exr_free(alloc, buf);
    return rc;
}

/* Open a mid-level reader on a file path: slurp the file into an allocator
 * buffer the reader then owns (freed by exr_reader_close). */
exr_result exr_reader_open_file(const char *path, const exr_allocator *alloc,
                               exr_reader **out) {
    FILE *fp;
    long sz;
    size_t n;
    uint8_t *buf;
    exr_result rc;

    if (!path || !out) return EXR_ERROR_INVALID_ARGUMENT;
    if (!alloc) alloc = exr_default_allocator();

    fp = fopen(path, "rb");
    if (!fp) return EXR_ERROR_IO;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return EXR_ERROR_IO; }
    sz = ftell(fp);
    if (sz < 0) { fclose(fp); return EXR_ERROR_IO; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return EXR_ERROR_IO; }
    buf = (uint8_t *)exr_malloc(alloc, (size_t)sz);
    if (!buf) { fclose(fp); return EXR_ERROR_OUT_OF_MEMORY; }
    n = fread(buf, 1, (size_t)sz, fp);
    fclose(fp);
    if (n != (size_t)sz) { exr_free(alloc, buf); return EXR_ERROR_IO; }

    rc = exr_reader_open_memory(buf, (size_t)sz, alloc, out);
    if (!EXR_OK(rc)) { exr_free(alloc, buf); return rc; }
    (*out)->free_mem = 1; /* hand buffer ownership to the reader */
    return EXR_SUCCESS;
}

/* ---- whole-file save --------------------------------------------------- */

static exr_result write_all(const char *path, const void *data, size_t size) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return EXR_ERROR_IO;
    if (size && fwrite(data, 1, size, fp) != size) {
        fclose(fp);
        return EXR_ERROR_IO;
    }
    if (fclose(fp) != 0) return EXR_ERROR_IO;
    return EXR_SUCCESS;
}

exr_result exr_save_to_file(const char *path, const exr_image *img,
                            exr_compression compression) {
    void *data = NULL;
    size_t size = 0;
    exr_result rc;
    const exr_allocator *a = exr_default_allocator();

    if (!path || !img) return EXR_ERROR_INVALID_ARGUMENT;
    rc = exr_save_to_memory(&data, &size, a, img, compression);
    if (!EXR_OK(rc)) return rc;
    rc = write_all(path, data, size);
    exr_free(a, data);
    return rc;
}

exr_result exr_writer_finalize_to_file(exr_writer *w, const char *path) {
    void *data = NULL;
    size_t size = 0;
    exr_result rc;
    if (!w || !path) return EXR_ERROR_INVALID_ARGUMENT;
    rc = exr_writer_finalize_to_memory(w, &data, &size);
    if (!EXR_OK(rc)) return rc;
    rc = write_all(path, data, size);
    exr_free(exr_writer_allocator(w), data); /* freed with the writer allocator */
    return rc;
}

/* ---- file-backed streaming sink ---------------------------------------- */

static exr_result file_sink_write(void *user, const void *data, size_t len) {
    FILE *fp = (FILE *)user;
    if (len && fwrite(data, 1, len, fp) != len) return EXR_ERROR_IO;
    return EXR_SUCCESS;
}
static exr_result file_sink_seek(void *user, uint64_t off) {
    FILE *fp = (FILE *)user;
    if (fseek(fp, (long)off, SEEK_SET) != 0) return EXR_ERROR_IO;
    return EXR_SUCCESS;
}
static exr_result file_sink_close(void *user) {
    FILE *fp = (FILE *)user;
    return (fclose(fp) == 0) ? EXR_SUCCESS : EXR_ERROR_IO;
}

exr_result exr_writer_begin_stream_file(exr_writer *w, const char *path,
                                        exr_compression comp) {
    exr_data_sink sink;
    exr_result rc;
    FILE *fp;
    if (!w || !path) return EXR_ERROR_INVALID_ARGUMENT;
    fp = fopen(path, "wb+");
    if (!fp) return EXR_ERROR_IO;
    sink.user = fp;
    sink.write = file_sink_write;
    sink.seek = file_sink_seek;
    sink.close = file_sink_close; /* the writer calls this once at end_stream */
    rc = exr_writer_begin_stream(w, &sink, comp);
    if (!EXR_OK(rc)) fclose(fp); /* ownership not transferred on failure */
    return rc;
}
