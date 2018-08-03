/*
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "zip.h"
#include "miniz.h"

#include <errno.h>
#include <sys/stat.h>
#include <time.h>

#if defined _WIN32 || defined __WIN32__
/* Win32, DOS */
#include <direct.h>

#define MKDIR(DIRNAME) _mkdir(DIRNAME)
#define STRCLONE(STR) ((STR) ? _strdup(STR) : NULL)
#define HAS_DEVICE(P)                                                          \
    ((((P)[0] >= 'A' && (P)[0] <= 'Z') || ((P)[0] >= 'a' && (P)[0] <= 'z')) && \
     (P)[1] == ':')
#define FILESYSTEM_PREFIX_LEN(P) (HAS_DEVICE(P) ? 2 : 0)
#define ISSLASH(C) ((C) == '/' || (C) == '\\')

#else
#define MKDIR(DIRNAME) mkdir(DIRNAME, 0755)
#define STRCLONE(STR) ((STR) ? strdup(STR) : NULL)
#endif

#ifndef FILESYSTEM_PREFIX_LEN
#define FILESYSTEM_PREFIX_LEN(P) 0
#endif

#ifndef ISSLASH
#define ISSLASH(C) ((C) == '/')
#endif

#define CLEANUP(ptr)           \
    do {                       \
        if (ptr) {             \
            free((void *)ptr); \
            ptr = NULL;        \
        }                      \
    } while (0)

static char *basename(const char *name) {
    char const *p;
    char const *base = name += FILESYSTEM_PREFIX_LEN(name);
    int all_slashes = 1;

    for (p = name; *p; p++) {
        if (ISSLASH(*p))
            base = p + 1;
        else
            all_slashes = 0;
    }

    /* If NAME is all slashes, arrange to return `/'. */
    if (*base == '\0' && ISSLASH(*name) && all_slashes) --base;

    return (char *)base;
}

static int mkpath(const char *path) {
    char const *p;
    char npath[MAX_PATH + 1] = {0};
    int len = 0;

    for (p = path; *p && len < MAX_PATH; p++) {
        if (ISSLASH(*p) && len > 0) {
            if (MKDIR(npath) == -1)
                if (errno != EEXIST) return -1;
        }
        npath[len++] = *p;
    }

    return 0;
}

static char *strrpl(const char *str, char oldchar, char newchar) {
    char *rpl = (char *)malloc(sizeof(char) * (1 + strlen(str)));
    char *begin = rpl;
    char c;
    while((c = *str++)) {
        if (c == oldchar) {
            c = newchar;
        }
        *rpl++ = c;
    }
    *rpl = '\0';

    return begin;
}

struct zip_entry_t {
    int index;
    const char *name;
    mz_uint64 uncomp_size;
    mz_uint64 comp_size;
    mz_uint32 uncomp_crc32;
    mz_uint64 offset;
    mz_uint8 header[MZ_ZIP_LOCAL_DIR_HEADER_SIZE];
    mz_uint64 header_offset;
    mz_uint16 method;
    mz_zip_writer_add_state state;
    tdefl_compressor comp;
};

struct zip_t {
    mz_zip_archive archive;
    mz_uint level;
    struct zip_entry_t entry;
    char mode;
};

struct zip_t *zip_open(const char *zipname, int level, char mode) {
    struct zip_t *zip = NULL;

    if (!zipname || strlen(zipname) < 1) {
        // zip_t archive name is empty or NULL
        goto cleanup;
    }

    if (level < 0) level = MZ_DEFAULT_LEVEL;
    if ((level & 0xF) > MZ_UBER_COMPRESSION) {
        // Wrong compression level
        goto cleanup;
    }

    zip = (struct zip_t *)calloc((size_t)1, sizeof(struct zip_t));
    if (!zip) goto cleanup;

    zip->level = level;
    zip->mode = mode;
    switch (mode) {
        case 'w':
            // Create a new archive.
            if (!mz_zip_writer_init_file(&(zip->archive), zipname, 0)) {
                // Cannot initialize zip_archive writer
                goto cleanup;
            }
            break;

        case 'r':
        case 'a':
            if (!mz_zip_reader_init_file(
                    &(zip->archive), zipname,
                    level | MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY)) {
                // An archive file does not exist or cannot initialize
                // zip_archive reader
                goto cleanup;
            }

            if (mode == 'a' &&
                !mz_zip_writer_init_from_reader(&(zip->archive), zipname)) {
                mz_zip_reader_end(&(zip->archive));
                goto cleanup;
            }

            break;

        default:
            goto cleanup;
    }

    return zip;

cleanup:
    CLEANUP(zip);
    return NULL;
}

void zip_close(struct zip_t *zip) {
    if (zip) {
        // Always finalize, even if adding failed for some reason, so we have a
        // valid central directory.
        mz_zip_writer_finalize_archive(&(zip->archive));

        mz_zip_writer_end(&(zip->archive));
        mz_zip_reader_end(&(zip->archive));

        CLEANUP(zip);
    }
}

int zip_entry_open(struct zip_t *zip, const char *entryname) {
    char *locname = NULL;
    size_t entrylen = 0;
    mz_zip_archive *pzip = NULL;
    mz_uint num_alignment_padding_bytes, level;

    if (!zip || !entryname) {
        return -1;
    }

    entrylen = strlen(entryname);
    if (entrylen < 1) {
        return -1;
    }

    pzip = &(zip->archive);
    /*
      .ZIP File Format Specification Version: 6.3.3

      4.4.17.1 The name of the file, with optional relative path.
      The path stored MUST not contain a drive or
      device letter, or a leading slash.  All slashes
      MUST be forward slashes '/' as opposed to
      backwards slashes '\' for compatibility with Amiga
      and UNIX file systems etc.  If input came from standard
      input, there is no file name field.
    */
    locname = strrpl(entryname, '\\', '/');

    if (zip->mode == 'r') {
        zip->entry.index = mz_zip_reader_locate_file(pzip, locname, NULL, 0);
        CLEANUP(locname);
        return (zip->entry.index < 0) ? -1 : 0;
    }

    zip->entry.index = zip->archive.m_total_files;
    zip->entry.name = locname;
    if (!zip->entry.name) {
        // Cannot parse zip entry name
        return -1;
    }

    zip->entry.comp_size = 0;
    zip->entry.uncomp_size = 0;
    zip->entry.uncomp_crc32 = MZ_CRC32_INIT;
    zip->entry.offset = zip->archive.m_archive_size;
    zip->entry.header_offset = zip->archive.m_archive_size;
    memset(zip->entry.header, 0,
           MZ_ZIP_LOCAL_DIR_HEADER_SIZE * sizeof(mz_uint8));
    zip->entry.method = 0;

    num_alignment_padding_bytes =
        mz_zip_writer_compute_padding_needed_for_file_alignment(pzip);

    if (!pzip->m_pState || (pzip->m_zip_mode != MZ_ZIP_MODE_WRITING)) {
        // Wrong zip mode
        return -1;
    }
    if (zip->level & MZ_ZIP_FLAG_COMPRESSED_DATA) {
        // Wrong zip compression level
        return -1;
    }
    // no zip64 support yet
    if ((pzip->m_total_files == 0xFFFF) ||
        ((pzip->m_archive_size + num_alignment_padding_bytes +
          MZ_ZIP_LOCAL_DIR_HEADER_SIZE + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE +
          entrylen) > 0xFFFFFFFF)) {
        // No zip64 support yet
        return -1;
    }
    if (!mz_zip_writer_write_zeros(
            pzip, zip->entry.offset,
            num_alignment_padding_bytes + sizeof(zip->entry.header))) {
        // Cannot memset zip entry header
        return -1;
    }

    zip->entry.header_offset += num_alignment_padding_bytes;
    if (pzip->m_file_offset_alignment) {
        MZ_ASSERT((zip->entry.header_offset &
                   (pzip->m_file_offset_alignment - 1)) == 0);
    }
    zip->entry.offset +=
        num_alignment_padding_bytes + sizeof(zip->entry.header);

    if (pzip->m_pWrite(pzip->m_pIO_opaque, zip->entry.offset, zip->entry.name,
                       entrylen) != entrylen) {
        // Cannot write data to zip entry
        return -1;
    }

    zip->entry.offset += entrylen;
    level = zip->level & 0xF;
    if (level) {
        zip->entry.state.m_pZip = pzip;
        zip->entry.state.m_cur_archive_file_ofs = zip->entry.offset;
        zip->entry.state.m_comp_size = 0;

        if (tdefl_init(&(zip->entry.comp), mz_zip_writer_add_put_buf_callback,
                       &(zip->entry.state),
                       tdefl_create_comp_flags_from_zip_params(
                           level, -15, MZ_DEFAULT_STRATEGY)) !=
            TDEFL_STATUS_OKAY) {
            // Cannot initialize the zip compressor
            return -1;
        }
    }

    return 0;
}

int zip_entry_close(struct zip_t *zip) {
    mz_zip_archive *pzip = NULL;
    mz_uint level;
    tdefl_status done;
    mz_uint16 entrylen;
    time_t t;
    struct tm *tm;
    mz_uint16 dos_time, dos_date;
    int status = -1;

    if (!zip) {
        // zip_t handler is not initialized
        return -1;
    }

    if (zip->mode == 'r') {
        return 0;
    }

    pzip = &(zip->archive);
    level = zip->level & 0xF;
    if (level) {
        done = tdefl_compress_buffer(&(zip->entry.comp), "", 0, TDEFL_FINISH);
        if (done != TDEFL_STATUS_DONE && done != TDEFL_STATUS_OKAY) {
            // Cannot flush compressed buffer
            goto cleanup;
        }
        zip->entry.comp_size = zip->entry.state.m_comp_size;
        zip->entry.offset = zip->entry.state.m_cur_archive_file_ofs;
        zip->entry.method = MZ_DEFLATED;
    }

    entrylen = (mz_uint16)strlen(zip->entry.name);
    t = time(NULL);
    tm = localtime(&t);
    dos_time = (mz_uint16)(((tm->tm_hour) << 11) + ((tm->tm_min) << 5) +
                           ((tm->tm_sec) >> 1));
    dos_date = (mz_uint16)(((tm->tm_year + 1900 - 1980) << 9) +
                           ((tm->tm_mon + 1) << 5) + tm->tm_mday);

    // no zip64 support yet
    if ((zip->entry.comp_size > 0xFFFFFFFF) ||
        (zip->entry.offset > 0xFFFFFFFF)) {
        // No zip64 support, yet
        goto cleanup;
    }

    if (!mz_zip_writer_create_local_dir_header(
            pzip, zip->entry.header, entrylen, 0, zip->entry.uncomp_size,
            zip->entry.comp_size, zip->entry.uncomp_crc32, zip->entry.method, 0,
            dos_time, dos_date)) {
        // Cannot create zip entry header
        goto cleanup;
    }

    if (pzip->m_pWrite(pzip->m_pIO_opaque, zip->entry.header_offset,
                       zip->entry.header, sizeof(zip->entry.header)) !=
        sizeof(zip->entry.header)) {
        // Cannot write zip entry header
        goto cleanup;
    }

    if (!mz_zip_writer_add_to_central_dir(
            pzip, zip->entry.name, entrylen, NULL, 0, "", 0,
            zip->entry.uncomp_size, zip->entry.comp_size,
            zip->entry.uncomp_crc32, zip->entry.method, 0, dos_time, dos_date,
            zip->entry.header_offset, 0)) {
        // Cannot write to zip central dir
        goto cleanup;
    }

    pzip->m_total_files++;
    pzip->m_archive_size = zip->entry.offset;
    status = 0;

cleanup:
    CLEANUP(zip->entry.name);
    return status;
}

int zip_entry_write(struct zip_t *zip, const void *buf, size_t bufsize) {
    mz_uint level;
    mz_zip_archive *pzip = NULL;
    tdefl_status status;

    if (!zip) {
        // zip_t handler is not initialized
        return -1;
    }

    pzip = &(zip->archive);
    if (buf && bufsize > 0) {
        zip->entry.uncomp_size += bufsize;
        zip->entry.uncomp_crc32 = (mz_uint32)mz_crc32(
            zip->entry.uncomp_crc32, (const mz_uint8 *)buf, bufsize);

        level = zip->level & 0xF;
        if (!level) {
            if ((pzip->m_pWrite(pzip->m_pIO_opaque, zip->entry.offset, buf,
                                bufsize) != bufsize)) {
                // Cannot write buffer
                return -1;
            }
            zip->entry.offset += bufsize;
            zip->entry.comp_size += bufsize;
        } else {
            status = tdefl_compress_buffer(&(zip->entry.comp), buf, bufsize,
                                           TDEFL_NO_FLUSH);
            if (status != TDEFL_STATUS_DONE && status != TDEFL_STATUS_OKAY) {
                // Cannot compress buffer
                return -1;
            }
        }
    }

    return 0;
}

int zip_entry_fwrite(struct zip_t *zip, const char *filename) {
    int status = 0;
    size_t n = 0;
    FILE *stream = NULL;
    mz_uint8 buf[MZ_ZIP_MAX_IO_BUF_SIZE] = {0};

    if (!zip) {
        // zip_t handler is not initialized
        return -1;
    }

    stream = fopen(filename, "rb");
    if (!stream) {
        // Cannot open filename
        return -1;
    }

    while ((n = fread(buf, sizeof(mz_uint8), MZ_ZIP_MAX_IO_BUF_SIZE, stream)) >
           0) {
        if (zip_entry_write(zip, buf, n) < 0) {
            status = -1;
            break;
        }
    }
    fclose(stream);

    return status;
}

int zip_entry_read(struct zip_t *zip, void **buf, size_t *bufsize) {
    mz_zip_archive *pzip = NULL;
    mz_uint idx;

    if (!zip) {
        // zip_t handler is not initialized
        return -1;
    }

    if (zip->mode != 'r' || zip->entry.index < 0) {
        // the entry is not found or we do not have read access
        return -1;
    }

    pzip = &(zip->archive);
    idx = (mz_uint)zip->entry.index;
    if (mz_zip_reader_is_file_a_directory(pzip, idx)) {
        // the entry is a directory
        return -1;
    }

    *buf = mz_zip_reader_extract_to_heap(pzip, idx, bufsize, 0);
    return (*buf) ? 0 : -1;
}

int zip_entry_fread(struct zip_t *zip, const char *filename) {
    mz_zip_archive *pzip = NULL;
    mz_uint idx;

    if (!zip) {
        // zip_t handler is not initialized
        return -1;
    }

    if (zip->mode != 'r' || zip->entry.index < 0) {
        // the entry is not found or we do not have read access
        return -1;
    }

    pzip = &(zip->archive);
    idx = (mz_uint)zip->entry.index;
    if (mz_zip_reader_is_file_a_directory(pzip, idx)) {
        // the entry is a directory
        return -1;
    }

    return (mz_zip_reader_extract_to_file(pzip, idx, filename, 0)) ? 0 : -1;
}

int zip_entry_extract(struct zip_t *zip,
                      size_t (*on_extract)(void *arg, unsigned long long offset,
                                           const void *buf, size_t bufsize),
                      void *arg) {
    mz_zip_archive *pzip = NULL;
    mz_uint idx;

    if (!zip) {
        // zip_t handler is not initialized
        return -1;
    }

    if (zip->mode != 'r' || zip->entry.index < 0) {
        // the entry is not found or we do not have read access
        return -1;
    }

    pzip = &(zip->archive);
    idx = (mz_uint)zip->entry.index;

    return (mz_zip_reader_extract_to_callback(pzip, idx, on_extract, arg, 0))
               ? 0
               : -1;
}

int zip_create(const char *zipname, const char *filenames[], size_t len) {
    int status = 0;
    size_t i;
    mz_zip_archive zip_archive;

    if (!zipname || strlen(zipname) < 1) {
        // zip_t archive name is empty or NULL
        return -1;
    }

    // Create a new archive.
    if (!memset(&(zip_archive), 0, sizeof(zip_archive))) {
        // Cannot memset zip archive
        return -1;
    }

    if (!mz_zip_writer_init_file(&zip_archive, zipname, 0)) {
        // Cannot initialize zip_archive writer
        return -1;
    }

    for (i = 0; i < len; ++i) {
        const char *name = filenames[i];
        if (!name) {
            status = -1;
            break;
        }

        if (!mz_zip_writer_add_file(&zip_archive, basename(name), name, "", 0,
                                    ZIP_DEFAULT_COMPRESSION_LEVEL)) {
            // Cannot add file to zip_archive
            status = -1;
            break;
        }
    }

    mz_zip_writer_finalize_archive(&zip_archive);
    mz_zip_writer_end(&zip_archive);
    return status;
}

int zip_extract(const char *zipname, const char *dir,
                int (*on_extract)(const char *filename, void *arg), void *arg) {
    int status = -1;
    mz_uint i, n;
    char path[MAX_PATH + 1] = {0};
    mz_zip_archive zip_archive;
    mz_zip_archive_file_stat info;
    size_t dirlen = 0;

    if (!memset(&(zip_archive), 0, sizeof(zip_archive))) {
        // Cannot memset zip archive
        return -1;
    }

    if (!zipname || !dir) {
        // Cannot parse zip archive name
        return -1;
    }

    dirlen = strlen(dir);
    if (dirlen + 1 > MAX_PATH) {
        return -1;
    }

    // Now try to open the archive.
    if (!mz_zip_reader_init_file(&zip_archive, zipname, 0)) {
        // Cannot initialize zip_archive reader
        return -1;
    }

    strcpy(path, dir);
    if (!ISSLASH(path[dirlen - 1])) {
#if defined _WIN32 || defined __WIN32__
        path[dirlen] = '\\';
#else
        path[dirlen] = '/';
#endif
        ++dirlen;
    }

    // Get and print information about each file in the archive.
    n = mz_zip_reader_get_num_files(&zip_archive);
    for (i = 0; i < n; ++i) {
        if (!mz_zip_reader_file_stat(&zip_archive, i, &info)) {
            // Cannot get information about zip archive;
            goto out;
        }
        strncpy(&path[dirlen], info.m_filename, MAX_PATH - dirlen);
        if (mkpath(path) < 0) {
            // Cannot make a path
            goto out;
        }

        if (!mz_zip_reader_is_file_a_directory(&zip_archive, i)) {
            if (!mz_zip_reader_extract_to_file(&zip_archive, i, path, 0)) {
                // Cannot extract zip archive to file
                goto out;
            }
        }

        if (on_extract) {
            if (on_extract(path, arg) < 0) {
                goto out;
            }
        }
    }
    status = 0;

out:
    // Close the archive, freeing any resources it was using
    if (!mz_zip_reader_end(&zip_archive)) {
        // Cannot end zip reader
        status = -1;
    }

    return status;
}
