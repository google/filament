/*
 * tocio - hosted stdio convenience layer (the ONLY <stdio.h> translation unit;
 * excluded from the freestanding core). Loads a config from a path and installs
 * a default file reader that resolves FileTransform LUTs relative to the config
 * directory.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

#include <stdio.h>

static toc_result read_whole_file(const char *path, const toc_allocator *a,
                                  char **out, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    long sz;
    char *buf;
    size_t got;
    if (!f) return TOC_ERROR_IO;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return TOC_ERROR_IO; }
    sz = ftell(f);
    if (sz < 0) { fclose(f); return TOC_ERROR_IO; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return TOC_ERROR_IO; }
    buf = (char *)toc_malloc(a, (size_t)sz + 1);
    if (!buf) { fclose(f); return TOC_ERROR_OUT_OF_MEMORY; }
    got = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (got != (size_t)sz) { toc_free(a, buf); return TOC_ERROR_IO; }
    buf[sz] = 0;
    *out = buf;
    *out_len = (size_t)sz;
    return TOC_SUCCESS;
}

/* file reader: user = config base directory (NUL-terminated, owned by arena) */
static toc_result default_reader(void *user, const char *name,
                                 const toc_allocator *a, char **data,
                                 size_t *len) {
    const char *dir = (const char *)user;
    char path[1024];
    size_t dl = dir ? strlen(dir) : 0, nl = strlen(name), p = 0;
    if (name[0] == '/' || dl == 0) {
        if (nl + 1 > sizeof(path)) return TOC_ERROR_IO;
        memcpy(path, name, nl + 1);
    } else {
        if (dl + 1 + nl + 1 > sizeof(path)) return TOC_ERROR_IO;
        memcpy(path, dir, dl); p = dl;
        path[p++] = '/';
        memcpy(path + p, name, nl + 1);
    }
    return read_whole_file(path, a, data, len);
}

toc_result toc_config_load_file(const char *path, const toc_allocator *a,
                                toc_config **out) {
    char *text = NULL;
    size_t len = 0;
    toc_result rc;
    if (!path || !out) return TOC_ERROR_INVALID_ARGUMENT;
    if (!a) a = toc_default_allocator();
    rc = read_whole_file(path, a, &text, &len);
    if (!TOC_OK(rc)) return rc;
    rc = toc_config_parse(text, len, a, out);
    toc_free(a, text);
    if (!TOC_OK(rc)) return rc;
    /* install a reader rooted at the config's directory */
    {
        const char *slash = NULL, *p = path;
        char *dir;
        for (; *p; ++p)
            if (*p == '/') slash = p;
        if (slash) {
            dir = toc_arena_strndup(&(*out)->ar, path, (size_t)(slash - path));
        } else {
            dir = toc_arena_strndup(&(*out)->ar, ".", 1);
        }
        toc_config_set_file_reader(*out, default_reader, dir);
    }
    return TOC_SUCCESS;
}
