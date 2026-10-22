
/**
 * Automated SDL_RWops test.
 *
 * Original code written by Edgar Simo "bobbens"
 * Ported by Markus Kauppila (markus.kauppila@gmail.com)
 * Updated and extended for SDL_test by aschiffler at ferzkopp dot net
 *
 * Released under Public Domain.
 */

/* quiet windows compiler warnings */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

#include "SDL.h"
#include "SDL_test.h"

/* ================= Test Case Implementation ================== */

const char* RWopsReadTestFilename = "rwops_read";
const char* RWopsWriteTestFilename = "rwops_write";
const char* RWopsAlphabetFilename = "rwops_alphabet";

static const char RWopsHelloWorldTestString[] = "Hello World!";
static const char RWopsHelloWorldCompString[] = "Hello World!";
static const char RWopsAlphabetString[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/* Fixture */

void
RWopsSetUp(void *arg)
{
    size_t fileLen;
    FILE *handle;
    size_t writtenLen;
    int result;

    /* Clean up from previous runs (if any); ignore errors */
    remove(RWopsReadTestFilename);
    remove(RWopsWriteTestFilename);
    remove(RWopsAlphabetFilename);

    /* Create a test file */
    handle = fopen(RWopsReadTestFilename, "w");
    SDLTest_AssertCheck(handle != NULL, "Verify creation of file '%s' returned non NULL handle", RWopsReadTestFilename);
        if (handle == NULL) return;

    /* Write some known text into it */
    fileLen = SDL_strlen(RWopsHelloWorldTestString);
    writtenLen = fwrite(RWopsHelloWorldTestString, 1, fileLen, handle);
    SDLTest_AssertCheck(fileLen == writtenLen, "Verify number of written bytes, expected %i, got %i", (int) fileLen, (int) writtenLen);
    result = fclose(handle);
    SDLTest_AssertCheck(result == 0, "Verify result from fclose, expected 0, got %i", result);

    /* Create a second test file */
    handle = fopen(RWopsAlphabetFilename, "w");
    SDLTest_AssertCheck(handle != NULL, "Verify creation of file '%s' returned non NULL handle", RWopsAlphabetFilename);
        if (handle == NULL) return;

    /* Write alphabet text into it */
    fileLen = SDL_strlen(RWopsAlphabetString);
    writtenLen = fwrite(RWopsAlphabetString, 1, fileLen, handle);
    SDLTest_AssertCheck(fileLen == writtenLen, "Verify number of written bytes, expected %i, got %i", (int) fileLen, (int) writtenLen);
    result = fclose(handle);
    SDLTest_AssertCheck(result == 0, "Verify result from fclose, expected 0, got %i", result);

    SDLTest_AssertPass("Creation of test file completed");
}

void
RWopsTearDown(void *arg)
{
    int result;

    /* Remove the created files to clean up; ignore errors for write filename */
    result = remove(RWopsReadTestFilename);
    SDLTest_AssertCheck(result == 0, "Verify result from remove(%s), expected 0, got %i", RWopsReadTestFilename, result);
    remove(RWopsWriteTestFilename);
    result = remove(RWopsAlphabetFilename);
    SDLTest_AssertCheck(result == 0, "Verify result from remove(%s), expected 0, got %i", RWopsAlphabetFilename, result);

    SDLTest_AssertPass("Cleanup of test files completed");
}

/**
 * @brief Makes sure parameters work properly. Local helper function.
 *
 * \sa
 * http://wiki.libsdl.org/moin.cgi/SDL_RWseek
 * http://wiki.libsdl.org/moin.cgi/SDL_RWread
 */
void
_testGenericRWopsValidations(SDL_RWops *rw, int write)
{
   char buf[sizeof(RWopsHelloWorldTestString)];
   Sint64 i;
   size_t s;
   int seekPos = SDLTest_RandomIntegerInRange(4, 8);

   /* Clear buffer */
   SDL_zero(buf);

   /* Set to start. */
   i = SDL_RWseek(rw, 0, RW_SEEK_SET );
   SDLTest_AssertPass("Call to SDL_RWseek succeeded");
   SDLTest_AssertCheck(i == (Sint64)0, "Verify seek to 0 with SDL_RWseek (RW_SEEK_SET), expected 0, got %"SDL_PRIs64, i);

   /* Test write. */
   s = SDL_RWwrite(rw, RWopsHelloWorldTestString, sizeof(RWopsHelloWorldTestString)-1, 1);
   SDLTest_AssertPass("Call to SDL_RWwrite succeeded");
   if (write) {
        SDLTest_AssertCheck(s == (size_t)1, "Verify result of writing one byte with SDL_RWwrite, expected 1, got %i", (int) s);
   }
   else {
        SDLTest_AssertCheck(s == (size_t)0, "Verify result of writing with SDL_RWwrite, expected: 0, got %i", (int) s);
   }

   /* Test seek to random position */
   i = SDL_RWseek( rw, seekPos, RW_SEEK_SET );
   SDLTest_AssertPass("Call to SDL_RWseek succeeded");
   SDLTest_AssertCheck(i == (Sint64)seekPos, "Verify seek to %i with SDL_RWseek (RW_SEEK_SET), expected %i, got %"SDL_PRIs64, seekPos, seekPos, i);

   /* Test seek back to start */
   i = SDL_RWseek(rw, 0, RW_SEEK_SET );
   SDLTest_AssertPass("Call to SDL_RWseek succeeded");
   SDLTest_AssertCheck(i == (Sint64)0, "Verify seek to 0 with SDL_RWseek (RW_SEEK_SET), expected 0, got %"SDL_PRIs64, i);

   /* Test read */
   s = SDL_RWread( rw, buf, 1, sizeof(RWopsHelloWorldTestString)-1 );
   SDLTest_AssertPass("Call to SDL_RWread succeeded");
   SDLTest_AssertCheck(
       s == (size_t)(sizeof(RWopsHelloWorldTestString)-1),
       "Verify result from SDL_RWread, expected %i, got %i",
       (int) (sizeof(RWopsHelloWorldTestString)-1),
       (int) s);
   SDLTest_AssertCheck(
       SDL_memcmp(buf, RWopsHelloWorldTestString, sizeof(RWopsHelloWorldTestString)-1 ) == 0,
       "Verify read bytes match expected string, expected '%s', got '%s'", RWopsHelloWorldTestString, buf);

   /* More seek tests. */
   i = SDL_RWseek( rw, -4, RW_SEEK_CUR );
   SDLTest_AssertPass("Call to SDL_RWseek(...,-4,RW_SEEK_CUR) succeeded");
   SDLTest_AssertCheck(
       i == (Sint64)(sizeof(RWopsHelloWorldTestString)-5),
       "Verify seek to -4 with SDL_RWseek (RW_SEEK_CUR), expected %i, got %i",
       (int) (sizeof(RWopsHelloWorldTestString)-5),
       (int) i);

   i = SDL_RWseek( rw, -1, RW_SEEK_END );
   SDLTest_AssertPass("Call to SDL_RWseek(...,-1,RW_SEEK_END) succeeded");
   SDLTest_AssertCheck(
       i == (Sint64)(sizeof(RWopsHelloWorldTestString)-2),
       "Verify seek to -1 with SDL_RWseek (RW_SEEK_END), expected %i, got %i",
       (int) (sizeof(RWopsHelloWorldTestString)-2),
       (int) i);

   /* Invalid whence seek */
   i = SDL_RWseek( rw, 0, 999 );
   SDLTest_AssertPass("Call to SDL_RWseek(...,0,invalid_whence) succeeded");
   SDLTest_AssertCheck(
       i == (Sint64)(-1),
       "Verify seek with SDL_RWseek (invalid_whence); expected: -1, got %i",
       (int) i);
}

/* !
 * Negative test for SDL_RWFromFile parameters
 *
 * \sa http://wiki.libsdl.org/moin.cgi/SDL_RWFromFile
 *
 */
int
rwops_testParamNegative (void)
{
   SDL_RWops *rwops;

   /* These should all fail. */
   rwops = SDL_RWFromFile(NULL, NULL);
   SDLTest_AssertPass("Call to SDL_RWFromFile(NULL, NULL) succeeded");
   SDLTest_AssertCheck(rwops == NULL, "Verify SDL_RWFromFile(NULL, NULL) returns NULL");

   rwops = SDL_RWFromFile(NULL, "ab+");
   SDLTest_AssertPass("Call to SDL_RWFromFile(NULL, \"ab+\") succeeded");
   SDLTest_AssertCheck(rwops == NULL, "Verify SDL_RWFromFile(NULL, \"ab+\") returns NULL");

   rwops = SDL_RWFromFile(NULL, "sldfkjsldkfj");
   SDLTest_AssertPass("Call to SDL_RWFromFile(NULL, \"sldfkjsldkfj\") succeeded");
   SDLTest_AssertCheck(rwops == NULL, "Verify SDL_RWFromFile(NULL, \"sldfkjsldkfj\") returns NULL");

   rwops = SDL_RWFromFile("something", "");
   SDLTest_AssertPass("Call to SDL_RWFromFile(\"something\", \"\") succeeded");
   SDLTest_AssertCheck(rwops == NULL, "Verify SDL_RWFromFile(\"something\", \"\") returns NULL");

   rwops = SDL_RWFromFile("something", NULL);
   SDLTest_AssertPass("Call to SDL_RWFromFile(\"something\", NULL) succeeded");
   SDLTest_AssertCheck(rwops == NULL, "Verify SDL_RWFromFile(\"something\", NULL) returns NULL");

   rwops = SDL_RWFromMem((void *)NULL, 10);
   SDLTest_AssertPass("Call to SDL_RWFromMem(NULL, 10) succeeded");
   SDLTest_AssertCheck(rwops == NULL, "Verify SDL_RWFromMem(NULL, 10) returns NULL");

   rwops = SDL_RWFromMem((void *)RWopsAlphabetString, 0);
   SDLTest_AssertPass("Call to SDL_RWFromMem(data, 0) succeeded");
   SDLTest_AssertCheck(rwops == NULL, "Verify SDL_RWFromMem(data, 0) returns NULL");

   rwops = SDL_RWFromConstMem((const void *)RWopsAlphabetString, 0);
   SDLTest_AssertPass("Call to SDL_RWFromConstMem(data, 0) succeeded");
   SDLTest_AssertCheck(rwops == NULL, "Verify SDL_RWFromConstMem(data, 0) returns NULL");

   return TEST_COMPLETED;
}

/**
 * @brief Tests opening from memory.
 *
 * \sa http://wiki.libsdl.org/moin.cgi/SDL_RWFromMem
 * \sa http://wiki.libsdl.org/moin.cgi/SDL_RWClose
 */
int
rwops_testMem (void)
{
   char mem[sizeof(RWopsHelloWorldTestString)];
   SDL_RWops *rw;
   int result;

   /* Clear buffer */
   SDL_zero(mem);

   /* Open */
   rw = SDL_RWFromMem(mem, sizeof(RWopsHelloWorldTestString)-1);
   SDLTest_AssertPass("Call to SDL_RWFromMem() succeeded");
   SDLTest_AssertCheck(rw != NULL, "Verify opening memory with SDL_RWFromMem does not return NULL");

   /* Bail out if NULL */
   if (rw == NULL) return TEST_ABORTED;

   /* Check type */
   SDLTest_AssertCheck(rw->type == SDL_RWOPS_MEMORY, "Verify RWops type is SDL_RWOPS_MEMORY; expected: %d, got: %d", SDL_RWOPS_MEMORY, rw->type);

   /* Run generic tests */
   _testGenericRWopsValidations(rw, 1);

   /* Close */
   result = SDL_RWclose(rw);
   SDLTest_AssertPass("Call to SDL_RWclose() succeeded");
   SDLTest_AssertCheck(result == 0, "Verify result value is 0; got: %d", result);

   return TEST_COMPLETED;
}


/**
 * @brief Tests opening from memory.
 *
 * \sa
 * http://wiki.libsdl.org/moin.cgi/SDL_RWFromConstMem
 * http://wiki.libsdl.org/moin.cgi/SDL_RWClose
 */
int
rwops_testConstMem (void)
{
   SDL_RWops *rw;
   int result;

   /* Open handle */
   rw = SDL_RWFromConstMem( RWopsHelloWorldCompString, sizeof(RWopsHelloWorldCompString)-1 );
   SDLTest_AssertPass("Call to SDL_RWFromConstMem() succeeded");
   SDLTest_AssertCheck(rw != NULL, "Verify opening memory with SDL_RWFromConstMem does not return NULL");

   /* Bail out if NULL */
   if (rw == NULL) return TEST_ABORTED;

   /* Check type */
   SDLTest_AssertCheck(rw->type == SDL_RWOPS_MEMORY_RO, "Verify RWops type is SDL_RWOPS_MEMORY_RO; expected: %d, got: %d", SDL_RWOPS_MEMORY_RO, rw->type);

   /* Run generic tests */
   _testGenericRWopsValidations( rw, 0 );

   /* Close handle */
   result = SDL_RWclose(rw);
   SDLTest_AssertPass("Call to SDL_RWclose() succeeded");
   SDLTest_AssertCheck(result == 0, "Verify result value is 0; got: %d", result);

  return TEST_COMPLETED;
}


/**
 * @brief Tests reading from file.
 *
 * \sa
 * http://wiki.libsdl.org/moin.cgi/SDL_RWFromFile
 * http://wiki.libsdl.org/moin.cgi/SDL_RWClose
 */
int
rwops_testFileRead(void)
{
   SDL_RWops *rw;
   int result;

   /* Read test. */
   rw = SDL_RWFromFile(RWopsReadTestFilename, "r");
   SDLTest_AssertPass("Call to SDL_RWFromFile(..,\"r\") succeeded");
   SDLTest_AssertCheck(rw != NULL, "Verify opening file with SDL_RWFromFile in read mode does not return NULL");

   /* Bail out if NULL */
   if (rw == NULL) return TEST_ABORTED;

   /* Check type */
#if defined(__ANDROID__)
   SDLTest_AssertCheck(
      rw->type == SDL_RWOPS_STDFILE || rw->type == SDL_RWOPS_JNIFILE,
      "Verify RWops type is SDL_RWOPS_STDFILE or SDL_RWOPS_JNIFILE; expected: %d|%d, got: %d", SDL_RWOPS_STDFILE, SDL_RWOPS_JNIFILE, rw->type);
#elif defined(__WIN32__)
   SDLTest_AssertCheck(
      rw->type == SDL_RWOPS_WINFILE,
      "Verify RWops type is SDL_RWOPS_WINFILE; expected: %d, got: %d", SDL_RWOPS_WINFILE, rw->type);
#else
   SDLTest_AssertCheck(
      rw->type == SDL_RWOPS_STDFILE,
      "Verify RWops type is SDL_RWOPS_STDFILE; expected: %d, got: %d", SDL_RWOPS_STDFILE, rw->type);
#endif

   /* Run generic tests */
   _testGenericRWopsValidations( rw, 0 );

   /* Close handle */
   result = SDL_RWclose(rw);
   SDLTest_AssertPass("Call to SDL_RWclose() succeeded");
   SDLTest_AssertCheck(result == 0, "Verify result value is 0; got: %d", result);

   return TEST_COMPLETED;
}

/**
 * @brief Tests writing from file.
 *
 * \sa
 * http://wiki.libsdl.org/moin.cgi/SDL_RWFromFile
 * http://wiki.libsdl.org/moin.cgi/SDL_RWClose
 */
int
rwops_testFileWrite(void)
{
   SDL_RWops *rw;
   int result;

   /* Write test. */
   rw = SDL_RWFromFile(RWopsWriteTestFilename, "w+");
   SDLTest_AssertPass("Call to SDL_RWFromFile(..,\"w+\") succeeded");
   SDLTest_AssertCheck(rw != NULL, "Verify opening file with SDL_RWFromFile in write mode does not return NULL");

   /* Bail out if NULL */
   if (rw == NULL) return TEST_ABORTED;

   /* Check type */
#if defined(__ANDROID__)
   SDLTest_AssertCheck(
      rw->type == SDL_RWOPS_STDFILE || rw->type == SDL_RWOPS_JNIFILE,
      "Verify RWops type is SDL_RWOPS_STDFILE or SDL_RWOPS_JNIFILE; expected: %d|%d, got: %d", SDL_RWOPS_STDFILE, SDL_RWOPS_JNIFILE, rw->type);
#elif defined(__WIN32__)
   SDLTest_AssertCheck(
      rw->type == SDL_RWOPS_WINFILE,
      "Verify RWops type is SDL_RWOPS_WINFILE; expected: %d, got: %d", SDL_RWOPS_WINFILE, rw->type);
#else
   SDLTest_AssertCheck(
      rw->type == SDL_RWOPS_STDFILE,
      "Verify RWops type is SDL_RWOPS_STDFILE; expected: %d, got: %d", SDL_RWOPS_STDFILE, rw->type);
#endif

   /* Run generic tests */
   _testGenericRWopsValidations( rw, 1 );

   /* Close handle */
   result = SDL_RWclose(rw);
   SDLTest_AssertPass("Call to SDL_RWclose() succeeded");
   SDLTest_AssertCheck(result == 0, "Verify result value is 0; got: %d", result);

   return TEST_COMPLETED;
}


/**
 * @brief Tests reading from file handle
 *
 * \sa
 * http://wiki.libsdl.org/moin.cgi/SDL_RWFromFP
 * http://wiki.libsdl.org/moin.cgi/SDL_RWClose
 *
 */
int
rwops_testFPRead(void)
{
   FILE *fp;
   SDL_RWops *rw;
   int result;

   /* Run read tests. */
   fp = fopen(RWopsReadTestFilename, "r");
   SDLTest_AssertCheck(fp != NULL, "Verify handle from opening file '%s' in read mode is not NULL", RWopsReadTestFilename);

   /* Bail out if NULL */
   if (fp == NULL) return TEST_ABORTED;

   /* Open */
   rw = SDL_RWFromFP( fp, SDL_TRUE );
   SDLTest_AssertPass("Call to SDL_RWFromFP() succeeded");
   SDLTest_AssertCheck(rw != NULL, "Verify opening file with SDL_RWFromFP in read mode does not return NULL");

   /* Bail out if NULL */
   if (rw == NULL) {
     fclose(fp);
     return TEST_ABORTED;
   }

   /* Check type */
   SDLTest_AssertCheck(
      rw->type == SDL_RWOPS_STDFILE,
      "Verify RWops type is SDL_RWOPS_STDFILE; expected: %d, got: %d", SDL_RWOPS_STDFILE, rw->type);

   /* Run generic tests */
   _testGenericRWopsValidations( rw, 0 );

   /* Close handle - does fclose() */
   result = SDL_RWclose(rw);
   SDLTest_AssertPass("Call to SDL_RWclose() succeeded");
   SDLTest_AssertCheck(result == 0, "Verify result value is 0; got: %d", result);

   return TEST_COMPLETED;
}


/**
 * @brief Tests writing to file handle
 *
 * \sa
 * http://wiki.libsdl.org/moin.cgi/SDL_RWFromFP
 * http://wiki.libsdl.org/moin.cgi/SDL_RWClose
 *
 */
int
rwops_testFPWrite(void)
{
   FILE *fp;
   SDL_RWops *rw;
   int result;

   /* Run write tests. */
   fp = fopen(RWopsWriteTestFilename, "w+");
   SDLTest_AssertCheck(fp != NULL, "Verify handle from opening file '%s' in write mode is not NULL", RWopsWriteTestFilename);

   /* Bail out if NULL */
   if (fp == NULL) return TEST_ABORTED;

   /* Open */
   rw = SDL_RWFromFP( fp, SDL_TRUE );
   SDLTest_AssertPass("Call to SDL_RWFromFP() succeeded");
   SDLTest_AssertCheck(rw != NULL, "Verify opening file with SDL_RWFromFP in write mode does not return NULL");

   /* Bail out if NULL */
   if (rw == NULL) {
     fclose(fp);
     return TEST_ABORTED;
   }

   /* Check type */
   SDLTest_AssertCheck(
      rw->type == SDL_RWOPS_STDFILE,
      "Verify RWops type is SDL_RWOPS_STDFILE; expected: %d, got: %d", SDL_RWOPS_STDFILE, rw->type);

   /* Run generic tests */
   _testGenericRWopsValidations( rw, 1 );

   /* Close handle - does fclose() */
   result = SDL_RWclose(rw);
   SDLTest_AssertPass("Call to SDL_RWclose() succeeded");
   SDLTest_AssertCheck(result == 0, "Verify result value is 0; got: %d", result);

   return TEST_COMPLETED;
}

/**
 * @brief Tests alloc and free RW context.
 *
 * \sa http://wiki.libsdl.org/moin.cgi/SDL_AllocRW
 * \sa http://wiki.libsdl.org/moin.cgi/SDL_FreeRW
 */
int
rwops_testAllocFree (void)
{
   /* Allocate context */
   SDL_RWops *rw = SDL_AllocRW();
   SDLTest_AssertPass("Call to SDL_AllocRW() succeeded");
   SDLTest_AssertCheck(rw != NULL, "Validate result from SDL_AllocRW() is not NULL");
   if (rw==NULL) return TEST_ABORTED;

   /* Check type */
   SDLTest_AssertCheck(
      rw->type == SDL_RWOPS_UNKNOWN,
      "Verify RWops type is SDL_RWOPS_UNKNOWN; expected: %d, got: %d", SDL_RWOPS_UNKNOWN, rw->type);

   /* Free context again */
   SDL_FreeRW(rw);
   SDLTest_AssertPass("Call to SDL_FreeRW() succeeded");

   return TEST_COMPLETED;
}

/**
 * @brief Compare memory and file reads
 *
 * \sa http://wiki.libsdl.org/moin.cgi/SDL_RWFromMem
 * \sa http://wiki.libsdl.org/moin.cgi/SDL_RWFromFile
 */
int
rwops_testCompareRWFromMemWithRWFromFile(void)
{
   int slen = 26;
   char buffer_file[27];
   char buffer_mem[27];
   size_t rv_file;
   size_t rv_mem;
   Uint64 sv_file;
   Uint64 sv_mem;
   SDL_RWops* rwops_file;
   SDL_RWops* rwops_mem;
   int size;
   int result;


   for (size=5; size<10; size++)
   {
     /* Terminate buffer */
     buffer_file[slen] = 0;
     buffer_mem[slen] = 0;

     /* Read/seek from memory */
     rwops_mem = SDL_RWFromMem((void *)RWopsAlphabetString, slen);
     SDLTest_AssertPass("Call to SDL_RWFromMem()");
     rv_mem = SDL_RWread(rwops_mem, buffer_mem, size, 6);
     SDLTest_AssertPass("Call to SDL_RWread(mem, size=%d)", size);
     sv_mem = SDL_RWseek(rwops_mem, 0, SEEK_END);
     SDLTest_AssertPass("Call to SDL_RWseek(mem,SEEK_END)");
     result = SDL_RWclose(rwops_mem);
     SDLTest_AssertPass("Call to SDL_RWclose(mem)");
     SDLTest_AssertCheck(result == 0, "Verify result value is 0; got: %d", result);

     /* Read/see from file */
     rwops_file = SDL_RWFromFile(RWopsAlphabetFilename, "r");
     SDLTest_AssertPass("Call to SDL_RWFromFile()");
     rv_file = SDL_RWread(rwops_file, buffer_file, size, 6);
     SDLTest_AssertPass("Call to SDL_RWread(file, size=%d)", size);
     sv_file = SDL_RWseek(rwops_file, 0, SEEK_END);
     SDLTest_AssertPass("Call to SDL_RWseek(file,SEEK_END)");
     result = SDL_RWclose(rwops_file);
     SDLTest_AssertPass("Call to SDL_RWclose(file)");
     SDLTest_AssertCheck(result == 0, "Verify result value is 0; got: %d", result);

     /* Compare */
     SDLTest_AssertCheck(rv_mem == rv_file, "Verify returned read blocks matches for mem and file reads; got: rv_mem=%d rv_file=%d", (int) rv_mem, (int) rv_file);
     SDLTest_AssertCheck(sv_mem == sv_file, "Verify SEEK_END position matches for mem and file seeks; got: sv_mem=%d sv_file=%d", (int) sv_mem, (int) sv_file);
     SDLTest_AssertCheck(buffer_mem[slen] == 0, "Verify mem buffer termination; expected: 0, got: %d", buffer_mem[slen]);
     SDLTest_AssertCheck(buffer_file[slen] == 0, "Verify file buffer termination; expected: 0, got: %d", buffer_file[slen]);
     SDLTest_AssertCheck(
       SDL_strncmp(buffer_mem, RWopsAlphabetString, slen) == 0,
       "Verify mem buffer contain alphabet string; expected: %s, got: %s", RWopsAlphabetString, buffer_mem);
     SDLTest_AssertCheck(
       SDL_strncmp(buffer_file, RWopsAlphabetString, slen) == 0,
       "Verify file buffer contain alphabet string; expected: %s, got: %s", RWopsAlphabetString, buffer_file);
   }

   return TEST_COMPLETED;
}

/**
 * @brief Tests writing and reading from file using endian aware functions.
 *
 * \sa
 * http://wiki.libsdl.org/moin.cgi/SDL_RWFromFile
 * http://wiki.libsdl.org/moin.cgi/SDL_RWClose
 * http://wiki.libsdl.org/moin.cgi/SDL_ReadBE16
 * http://wiki.libsdl.org/moin.cgi/SDL_WriteBE16
 */
int
rwops_testFileWriteReadEndian(void)
{
   SDL_RWops *rw;
   Sint64 result;
   int mode;
   size_t objectsWritten;
   Uint16 BE16value;
   Uint32 BE32value;
   Uint64 BE64value;
   Uint16 LE16value;
   Uint32 LE32value;
   Uint64 LE64value;
   Uint16 BE16test;
   Uint32 BE32test;
   Uint64 BE64test;
   Uint16 LE16test;
   Uint32 LE32test;
   Uint64 LE64test;
   int cresult;

   for (mode = 0; mode < 3; mode++) {

     /* Create test data */
     switch (mode) {
       case 0:
        SDLTest_Log("All 0 values");
        BE16value = 0;
        BE32value = 0;
        BE64value = 0;
        LE16value = 0;
        LE32value = 0;
        LE64value = 0;
        break;
       case 1:
        SDLTest_Log("All 1 values");
        BE16value = 1;
        BE32value = 1;
        BE64value = 1;
        LE16value = 1;
        LE32value = 1;
        LE64value = 1;
        break;
       case 2:
        SDLTest_Log("Random values");
        BE16value = SDLTest_RandomUint16();
        BE32value = SDLTest_RandomUint32();
        BE64value = SDLTest_RandomUint64();
        LE16value = SDLTest_RandomUint16();
        LE32value = SDLTest_RandomUint32();
        LE64value = SDLTest_RandomUint64();
        break;
     }

     /* Write test. */
     rw = SDL_RWFromFile(RWopsWriteTestFilename, "w+");
     SDLTest_AssertPass("Call to SDL_RWFromFile(..,\"w+\")");
     SDLTest_AssertCheck(rw != NULL, "Verify opening file with SDL_RWFromFile in write mode does not return NULL");

     /* Bail out if NULL */
     if (rw == NULL) return TEST_ABORTED;

     /* Write test data */
     objectsWritten = SDL_WriteBE16(rw, BE16value);
     SDLTest_AssertPass("Call to SDL_WriteBE16()");
     SDLTest_AssertCheck(objectsWritten == 1, "Validate number of objects written, expected: 1, got: %i", (int) objectsWritten);
     objectsWritten = SDL_WriteBE32(rw, BE32value);
     SDLTest_AssertPass("Call to SDL_WriteBE32()");
     SDLTest_AssertCheck(objectsWritten == 1, "Validate number of objects written, expected: 1, got: %i", (int) objectsWritten);
     objectsWritten = SDL_WriteBE64(rw, BE64value);
     SDLTest_AssertPass("Call to SDL_WriteBE64()");
     SDLTest_AssertCheck(objectsWritten == 1, "Validate number of objects written, expected: 1, got: %i", (int) objectsWritten);
     objectsWritten = SDL_WriteLE16(rw, LE16value);
     SDLTest_AssertPass("Call to SDL_WriteLE16()");
     SDLTest_AssertCheck(objectsWritten == 1, "Validate number of objects written, expected: 1, got: %i", (int) objectsWritten);
     objectsWritten = SDL_WriteLE32(rw, LE32value);
     SDLTest_AssertPass("Call to SDL_WriteLE32()");
     SDLTest_AssertCheck(objectsWritten == 1, "Validate number of objects written, expected: 1, got: %i", (int) objectsWritten);
     objectsWritten = SDL_WriteLE64(rw, LE64value);
     SDLTest_AssertPass("Call to SDL_WriteLE64()");
     SDLTest_AssertCheck(objectsWritten == 1, "Validate number of objects written, expected: 1, got: %i", (int) objectsWritten);

     /* Test seek to start */
     result = SDL_RWseek( rw, 0, RW_SEEK_SET );
     SDLTest_AssertPass("Call to SDL_RWseek succeeded");
     SDLTest_AssertCheck(result == 0, "Verify result from position 0 with SDL_RWseek, expected 0, got %i", (int) result);

     /* Read test data */
     BE16test = SDL_ReadBE16(rw);
     SDLTest_AssertPass("Call to SDL_ReadBE16()");
     SDLTest_AssertCheck(BE16test == BE16value, "Validate return value from SDL_ReadBE16, expected: %hu, got: %hu", BE16value, BE16test);
     BE32test = SDL_ReadBE32(rw);
     SDLTest_AssertPass("Call to SDL_ReadBE32()");
     SDLTest_AssertCheck(BE32test == BE32value, "Validate return value from SDL_ReadBE32, expected: %u, got: %u", BE32value, BE32test);
     BE64test = SDL_ReadBE64(rw);
     SDLTest_AssertPass("Call to SDL_ReadBE64()");
     SDLTest_AssertCheck(BE64test == BE64value, "Validate return value from SDL_ReadBE64, expected: %"SDL_PRIu64", got: %"SDL_PRIu64, BE64value, BE64test);
     LE16test = SDL_ReadLE16(rw);
     SDLTest_AssertPass("Call to SDL_ReadLE16()");
     SDLTest_AssertCheck(LE16test == LE16value, "Validate return value from SDL_ReadLE16, expected: %hu, got: %hu", LE16value, LE16test);
     LE32test = SDL_ReadLE32(rw);
     SDLTest_AssertPass("Call to SDL_ReadLE32()");
     SDLTest_AssertCheck(LE32test == LE32value, "Validate return value from SDL_ReadLE32, expected: %u, got: %u", LE32value, LE32test);
     LE64test = SDL_ReadLE64(rw);
     SDLTest_AssertPass("Call to SDL_ReadLE64()");
     SDLTest_AssertCheck(LE64test == LE64value, "Validate return value from SDL_ReadLE64, expected: %"SDL_PRIu64", got: %"SDL_PRIu64, LE64value, LE64test);

     /* Close handle */
     cresult = SDL_RWclose(rw);
     SDLTest_AssertPass("Call to SDL_RWclose() succeeded");
     SDLTest_AssertCheck(cresult == 0, "Verify result value is 0; got: %d", cresult);
   }

   return TEST_COMPLETED;
}


/* ================= Test References ================== */

/* RWops test cases */
static const SDLTest_TestCaseReference rwopsTest1 =
        { (SDLTest_TestCaseFp)rwops_testParamNegative, "rwops_testParamNegative", "Negative test for SDL_RWFromFile parameters", TEST_ENABLED };

static const SDLTest_TestCaseReference rwopsTest2 =
        { (SDLTest_TestCaseFp)rwops_testMem, "rwops_testMem", "Tests opening from memory", TEST_ENABLED };

static const SDLTest_TestCaseReference rwopsTest3 =
        { (SDLTest_TestCaseFp)rwops_testConstMem, "rwops_testConstMem", "Tests opening from (const) memory", TEST_ENABLED };

static const SDLTest_TestCaseReference rwopsTest4 =
        { (SDLTest_TestCaseFp)rwops_testFileRead, "rwops_testFileRead", "Tests reading from a file", TEST_ENABLED };

static const SDLTest_TestCaseReference rwopsTest5 =
        { (SDLTest_TestCaseFp)rwops_testFileWrite, "rwops_testFileWrite", "Test writing to a file", TEST_ENABLED };

static const SDLTest_TestCaseReference rwopsTest6 =
        { (SDLTest_TestCaseFp)rwops_testFPRead, "rwops_testFPRead", "Test reading from file pointer", TEST_ENABLED };

static const SDLTest_TestCaseReference rwopsTest7 =
        { (SDLTest_TestCaseFp)rwops_testFPWrite, "rwops_testFPWrite", "Test writing to file pointer", TEST_ENABLED };

static const SDLTest_TestCaseReference rwopsTest8 =
        { (SDLTest_TestCaseFp)rwops_testAllocFree, "rwops_testAllocFree", "Test alloc and free of RW context", TEST_ENABLED };

static const SDLTest_TestCaseReference rwopsTest9 =
        { (SDLTest_TestCaseFp)rwops_testFileWriteReadEndian, "rwops_testFileWriteReadEndian", "Test writing and reading via the Endian aware functions", TEST_ENABLED };

static const SDLTest_TestCaseReference rwopsTest10 =
        { (SDLTest_TestCaseFp)rwops_testCompareRWFromMemWithRWFromFile, "rwops_testCompareRWFromMemWithRWFromFile", "Compare RWFromMem and RWFromFile RWops for read and seek", TEST_ENABLED };

/* Sequence of RWops test cases */
static const SDLTest_TestCaseReference *rwopsTests[] =  {
    &rwopsTest1, &rwopsTest2, &rwopsTest3, &rwopsTest4, &rwopsTest5, &rwopsTest6,
    &rwopsTest7, &rwopsTest8, &rwopsTest9, &rwopsTest10, NULL
};

/* RWops test suite (global) */
SDLTest_TestSuiteReference rwopsTestSuite = {
    "RWops",
    RWopsSetUp,
    rwopsTests,
    RWopsTearDown
};
