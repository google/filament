#include <zip.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ZIPNAME "test.zip"
#define TESTDATA1 "Some test data 1...\0"
#define TESTDATA2 "Some test data 2...\0"

static void test_write(void) {
    struct zip_t *zip = zip_open(ZIPNAME, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    assert(zip != NULL);

    assert(0 == zip_entry_open(zip, "test/test-1.txt"));
    assert(0 == zip_entry_write(zip, TESTDATA1, strlen(TESTDATA1)));
    assert(0 == zip_entry_close(zip));

    zip_close(zip);
}

static void test_append(void) {
    struct zip_t *zip = zip_open(ZIPNAME, ZIP_DEFAULT_COMPRESSION_LEVEL, 'a');
    assert(zip != NULL);

    assert(0 == zip_entry_open(zip, "test\\test-2.txt"));
    assert(0 == zip_entry_write(zip, TESTDATA2, strlen(TESTDATA2)));
    assert(0 == zip_entry_close(zip));

    zip_close(zip);
}

static void test_read(void) {
    char *buf = NULL;
    size_t bufsize;
    struct zip_t *zip = zip_open(ZIPNAME, 0, 'r');
    assert(zip != NULL);

    assert(0 == zip_entry_open(zip, "test\\test-1.txt"));
    assert(0 == zip_entry_read(zip, (void **)&buf, &bufsize));
    assert(bufsize == strlen(TESTDATA1));
    assert(0 == strncmp(buf, TESTDATA1, bufsize));
    assert(0 == zip_entry_close(zip));
    free(buf);
    buf = NULL;
    bufsize = 0;

    assert(0 == zip_entry_open(zip, "test/test-2.txt"));
    assert(0 == zip_entry_read(zip, (void **)&buf, &bufsize));
    assert(bufsize == strlen(TESTDATA2));
    assert(0 == strncmp(buf, TESTDATA2, bufsize));
    assert(0 == zip_entry_close(zip));
    free(buf);
    buf = NULL;
    bufsize = 0;

    zip_close(zip);
}

struct buffer_t {
    char *data;
    size_t size;
};

static size_t on_extract(void *arg, unsigned long long offset, const void *data,
                         size_t size) {
    struct buffer_t *buf = (struct buffer_t *)arg;
    buf->data = realloc(buf->data, buf->size + size + 1);
    assert(NULL != buf->data);

    memcpy(&(buf->data[buf->size]), data, size);
    buf->size += size;
    buf->data[buf->size] = 0;

    return size;
}

static void test_extract(void) {
    struct buffer_t buf = {0};

    struct zip_t *zip = zip_open(ZIPNAME, 0, 'r');
    assert(zip != NULL);

    assert(0 == zip_entry_open(zip, "test/test-1.txt"));
    assert(0 == zip_entry_extract(zip, on_extract, &buf));

    assert(buf.size == strlen(TESTDATA1));
    assert(0 == strncmp(buf.data, TESTDATA1, buf.size));
    assert(0 == zip_entry_close(zip));
    free(buf.data);
    buf.data = NULL;
    buf.size = 0;

    zip_close(zip);
}

int main(int argc, char *argv[]) {
    test_write();
    test_append();
    test_read();
    test_extract();

    return remove(ZIPNAME);
}
