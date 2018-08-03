/*
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once
#ifndef ZIP_H
#define ZIP_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_PATH
#define MAX_PATH 32767 /* # chars in a path name including NULL */
#endif

#define ZIP_DEFAULT_COMPRESSION_LEVEL 6

/*
  This data structure is used throughout the library to represent zip archive
  - forward declaration.
*/
struct zip_t;

/*
  Opens zip archive with compression level using the given mode.

  Args:
    zipname: zip archive file name.
    level: compression level (0-9 are the standard zlib-style levels).
    mode: file access mode.
        'r': opens a file for reading/extracting (the file must exists).
        'w': creates an empty file for writing.
        'a': appends to an existing archive.

  Returns:
    The zip archive handler or NULL on error
*/
extern struct zip_t *zip_open(const char *zipname, int level, char mode);

/*
  Closes zip archive, releases resources - always finalize.

  Args:
    zip: zip archive handler.
*/
extern void zip_close(struct zip_t *zip);

/*
  Opens a new entry for writing in a zip archive.

  Args:
    zip: zip archive handler.
    entryname: an entry name in local dictionary.

  Returns:
    The return code - 0 on success, negative number (< 0) on error.
*/
extern int zip_entry_open(struct zip_t *zip, const char *entryname);

/*
  Closes a zip entry, flushes buffer and releases resources.

  Args:
    zip: zip archive handler.

  Returns:
    The return code - 0 on success, negative number (< 0) on error.
*/
extern int zip_entry_close(struct zip_t *zip);

/*
  Compresses an input buffer for the current zip entry.

  Args:
    zip: zip archive handler.
    buf: input buffer.
    bufsize: input buffer size (in bytes).

  Returns:
    The return code - 0 on success, negative number (< 0) on error.
*/
extern int zip_entry_write(struct zip_t *zip, const void *buf, size_t bufsize);

/*
  Compresses a file for the current zip entry.

  Args:
    zip: zip archive handler.
    filename: input file.

  Returns:
    The return code - 0 on success, negative number (< 0) on error.
*/
extern int zip_entry_fwrite(struct zip_t *zip, const char *filename);

/*
  Extracts the current zip entry into output buffer.
  The function allocates sufficient memory for a output buffer.

  Args:
    zip: zip archive handler.
    buf: output buffer.
    bufsize: output buffer size (in bytes).

  Note:
    - remember to release memory allocated for a output buffer.
    - for large entries, please take a look at zip_entry_extract function.

  Returns:
    The return code - 0 on success, negative number (< 0) on error.
*/
extern int zip_entry_read(struct zip_t *zip, void **buf, size_t *bufsize);

/*
  Extracts the current zip entry into output file.

  Args:
    zip: zip archive handler.
    filename: output file.

  Returns:
    The return code - 0 on success, negative number (< 0) on error.
*/
extern int zip_entry_fread(struct zip_t *zip, const char *filename);

/*
   Extract the current zip entry using a callback function (on_extract).

   Args:
    zip: zip archive handler.
    on_extract: callback function.
    arg: opaque pointer (optional argument,
                         which you can pass to the on_extract callback)

   Returns:
    The return code - 0 on success, negative number (< 0) on error.
 */
extern int zip_entry_extract(struct zip_t *zip,
                             size_t (*on_extract)(void *arg,
                                                  unsigned long long offset,
                                                  const void *data,
                                                  size_t size),
                             void *arg);

/*
  Creates a new archive and puts files into a single zip archive.

  Args:
    zipname: zip archive file.
    filenames: input files.
    len: number of input files.

  Returns:
    The return code - 0 on success, negative number (< 0) on error.
*/
extern int zip_create(const char *zipname, const char *filenames[], size_t len);

/*
  Extracts a zip archive file into directory.

  If on_extract_entry is not NULL, the callback will be called after
  successfully extracted each zip entry.
  Returning a negative value from the callback will cause abort and return an
  error. The last argument (void *arg) is optional, which you can use to pass
  data to the on_extract_entry callback.

  Args:
    zipname: zip archive file.
    dir: output directory.
    on_extract_entry: on extract callback.
    arg: opaque pointer.

  Returns:
    The return code - 0 on success, negative number (< 0) on error.
*/
extern int zip_extract(const char *zipname, const char *dir,
                       int (*on_extract_entry)(const char *filename, void *arg),
                       void *arg);

#ifdef __cplusplus
}
#endif

#endif
