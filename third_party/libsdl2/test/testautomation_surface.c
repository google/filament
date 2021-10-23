/**
 * Original code: automated SDL surface test written by Edgar Simo "bobbens"
 * Adapted/rewritten for test lib by Andreas Schiffler
 */

/* Supress C4996 VS compiler warnings for unlink() */
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <sys/stat.h>

#include "SDL.h"
#include "SDL_test.h"

#ifdef __MACOSX__
#include <unistd.h> /* For unlink() */
#endif

/* ================= Test Case Implementation ================== */

/* Shared test surface */

static SDL_Surface *referenceSurface = NULL;
static SDL_Surface *testSurface = NULL;

/* Helper functions for the test cases */

#define TEST_SURFACE_WIDTH testSurface->w
#define TEST_SURFACE_HEIGHT testSurface->h

/* Fixture */

/* Create a 32-bit writable surface for blitting tests */
void
_surfaceSetUp(void *arg)
{
    int result;
    SDL_BlendMode blendMode = SDL_BLENDMODE_NONE;
    SDL_BlendMode currentBlendMode;
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    referenceSurface = SDLTest_ImageBlit(); /* For size info */
    testSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, referenceSurface->w, referenceSurface->h, 32, rmask, gmask, bmask, amask);
    SDLTest_AssertCheck(testSurface != NULL, "Check that testSurface is not NULL");
    if (testSurface != NULL) {
      /* Disable blend mode for target surface */
      result = SDL_SetSurfaceBlendMode(testSurface, blendMode);
      SDLTest_AssertCheck(result == 0, "Validate result from SDL_SetSurfaceBlendMode, expected: 0, got: %i", result);
      result = SDL_GetSurfaceBlendMode(testSurface, &currentBlendMode);
      SDLTest_AssertCheck(result == 0, "Validate result from SDL_GetSurfaceBlendMode, expected: 0, got: %i", result);
      SDLTest_AssertCheck(currentBlendMode == blendMode, "Validate blendMode, expected: %i, got: %i", blendMode, currentBlendMode);
    }
}

void
_surfaceTearDown(void *arg)
{
    SDL_FreeSurface(referenceSurface);
    referenceSurface = NULL;
    SDL_FreeSurface(testSurface);
    testSurface = NULL;
}

/**
 * Helper that clears the test surface
 */
void _clearTestSurface()
{
    int ret;
    Uint32 color;

    /* Clear surface. */
    color = SDL_MapRGBA( testSurface->format, 0, 0, 0, 0);
    SDLTest_AssertPass("Call to SDL_MapRGBA()");
    ret = SDL_FillRect( testSurface, NULL, color);
    SDLTest_AssertPass("Call to SDL_FillRect()");
    SDLTest_AssertCheck(ret == 0, "Verify result from SDL_FillRect, expected: 0, got: %i", ret);
}

/**
 * Helper that blits in a specific blend mode, -1 for basic blitting, -2 for color mod, -3 for alpha mod, -4 for mixed blend modes.
 */
void _testBlitBlendMode(int mode)
{
    int ret;
    int i, j, ni, nj;
    SDL_Surface *face;
    SDL_Rect rect;
    int nmode;
    SDL_BlendMode bmode;
    int checkFailCount1;
    int checkFailCount2;
    int checkFailCount3;
    int checkFailCount4;

    /* Check test surface */
    SDLTest_AssertCheck(testSurface != NULL, "Verify testSurface is not NULL");
    if (testSurface == NULL) return;

    /* Create sample surface */
    face = SDLTest_ImageFace();
    SDLTest_AssertCheck(face != NULL, "Verify face surface is not NULL");
    if (face == NULL) return;

        /* Reset alpha modulation */
    ret = SDL_SetSurfaceAlphaMod(face, 255);
    SDLTest_AssertPass("Call to SDL_SetSurfaceAlphaMod()");
    SDLTest_AssertCheck(ret == 0, "Verify result from SDL_SetSurfaceAlphaMod(), expected: 0, got: %i", ret);

        /* Reset color modulation */
    ret = SDL_SetSurfaceColorMod(face, 255, 255, 255);
    SDLTest_AssertPass("Call to SDL_SetSurfaceColorMod()");
    SDLTest_AssertCheck(ret == 0, "Verify result from SDL_SetSurfaceColorMod(), expected: 0, got: %i", ret);

        /* Reset color key */
    ret = SDL_SetColorKey(face, SDL_FALSE, 0);
    SDLTest_AssertPass("Call to SDL_SetColorKey()");
    SDLTest_AssertCheck(ret == 0, "Verify result from SDL_SetColorKey(), expected: 0, got: %i", ret);

    /* Clear the test surface */
        _clearTestSurface();

    /* Target rect size */
    rect.w = face->w;
    rect.h = face->h;

    /* Steps to take */
    ni = testSurface->w - face->w;
    nj = testSurface->h - face->h;

    /* Optionally set blend mode. */
    if (mode >= 0) {
        ret = SDL_SetSurfaceBlendMode( face, (SDL_BlendMode)mode );
        SDLTest_AssertPass("Call to SDL_SetSurfaceBlendMode()");
        SDLTest_AssertCheck(ret == 0, "Verify result from SDL_SetSurfaceBlendMode(..., %i), expected: 0, got: %i", mode, ret);
    }

    /* Test blend mode. */
    checkFailCount1 = 0;
    checkFailCount2 = 0;
    checkFailCount3 = 0;
    checkFailCount4 = 0;
    for (j=0; j <= nj; j+=4) {
      for (i=0; i <= ni; i+=4) {
        if (mode == -2) {
            /* Set color mod. */
            ret = SDL_SetSurfaceColorMod( face, (255/nj)*j, (255/ni)*i, (255/nj)*j );
            if (ret != 0) checkFailCount2++;
        }
        else if (mode == -3) {
            /* Set alpha mod. */
            ret = SDL_SetSurfaceAlphaMod( face, (255/ni)*i );
            if (ret != 0) checkFailCount3++;
        }
        else if (mode == -4) {
            /* Crazy blending mode magic. */
            nmode = (i/4*j/4) % 4;
            if (nmode==0) {
                bmode = SDL_BLENDMODE_NONE;
            } else if (nmode==1) {
                bmode = SDL_BLENDMODE_BLEND;
            } else if (nmode==2) {
                bmode = SDL_BLENDMODE_ADD;
            } else if (nmode==3) {
                bmode = SDL_BLENDMODE_MOD;
            }
            ret = SDL_SetSurfaceBlendMode( face, bmode );
            if (ret != 0) checkFailCount4++;
        }

         /* Blitting. */
         rect.x = i;
         rect.y = j;
         ret = SDL_BlitSurface( face, NULL, testSurface, &rect );
         if (ret != 0) checkFailCount1++;
      }
    }
    SDLTest_AssertCheck(checkFailCount1 == 0, "Validate results from calls to SDL_BlitSurface, expected: 0, got: %i", checkFailCount1);
    SDLTest_AssertCheck(checkFailCount2 == 0, "Validate results from calls to SDL_SetSurfaceColorMod, expected: 0, got: %i", checkFailCount2);
    SDLTest_AssertCheck(checkFailCount3 == 0, "Validate results from calls to SDL_SetSurfaceAlphaMod, expected: 0, got: %i", checkFailCount3);
    SDLTest_AssertCheck(checkFailCount4 == 0, "Validate results from calls to SDL_SetSurfaceBlendMode, expected: 0, got: %i", checkFailCount4);

    /* Clean up */
    SDL_FreeSurface(face);
    face = NULL;
}

/* Helper to check that a file exists */
void
_AssertFileExist(const char *filename)
{
    struct stat st;
    int ret = stat(filename, &st);

    SDLTest_AssertCheck(ret == 0, "Verify file '%s' exists", filename);
}


/* Test case functions */

/**
 * @brief Tests sprite saving and loading
 */
int
surface_testSaveLoadBitmap(void *arg)
{
    int ret;
    const char *sampleFilename = "testSaveLoadBitmap.bmp";
    SDL_Surface *face;
    SDL_Surface *rface;

    /* Create sample surface */
    face = SDLTest_ImageFace();
    SDLTest_AssertCheck(face != NULL, "Verify face surface is not NULL");
    if (face == NULL) return TEST_ABORTED;

    /* Delete test file; ignore errors */
    unlink(sampleFilename);

    /* Save a surface */
    ret = SDL_SaveBMP(face, sampleFilename);
    SDLTest_AssertPass("Call to SDL_SaveBMP()");
    SDLTest_AssertCheck(ret == 0, "Verify result from SDL_SaveBMP, expected: 0, got: %i", ret);
    _AssertFileExist(sampleFilename);

    /* Load a surface */
    rface = SDL_LoadBMP(sampleFilename);
    SDLTest_AssertPass("Call to SDL_LoadBMP()");
    SDLTest_AssertCheck(rface != NULL, "Verify result from SDL_LoadBMP is not NULL");
    if (rface != NULL) {
        SDLTest_AssertCheck(face->w == rface->w, "Verify width of loaded surface, expected: %i, got: %i", face->w, rface->w);
        SDLTest_AssertCheck(face->h == rface->h, "Verify height of loaded surface, expected: %i, got: %i", face->h, rface->h);
    }

    /* Delete test file; ignore errors */
    unlink(sampleFilename);

    /* Clean up */
    SDL_FreeSurface(face);
    face = NULL;
    SDL_FreeSurface(rface);
    rface = NULL;

    return TEST_COMPLETED;
}

/* !
 *  Tests surface conversion.
 */
int
surface_testSurfaceConversion(void *arg)
{
    SDL_Surface *rface = NULL, *face = NULL;
    int ret = 0;

    /* Create sample surface */
    face = SDLTest_ImageFace();
    SDLTest_AssertCheck(face != NULL, "Verify face surface is not NULL");
    if (face == NULL)
        return TEST_ABORTED;

    /* Set transparent pixel as the pixel at (0,0) */
    if (face->format->palette) {
       ret = SDL_SetColorKey(face, SDL_RLEACCEL, *(Uint8 *) face->pixels);
       SDLTest_AssertPass("Call to SDL_SetColorKey()");
       SDLTest_AssertCheck(ret == 0, "Verify result from SDL_SetColorKey, expected: 0, got: %i", ret);
    }

    /* Convert to 32 bit to compare. */
    rface = SDL_ConvertSurface( face, testSurface->format, 0 );
    SDLTest_AssertPass("Call to SDL_ConvertSurface()");
    SDLTest_AssertCheck(rface != NULL, "Verify result from SDL_ConvertSurface is not NULL");

    /* Compare surface. */
    ret = SDLTest_CompareSurfaces( rface, face, 0 );
    SDLTest_AssertCheck(ret == 0, "Validate result from SDLTest_CompareSurfaces, expected: 0, got: %i", ret);

    /* Clean up. */
    SDL_FreeSurface(face);
    face = NULL;
    SDL_FreeSurface(rface);
    rface = NULL;

    return TEST_COMPLETED;
}


/* !
 *  Tests surface conversion across all pixel formats.
 */
int
surface_testCompleteSurfaceConversion(void *arg)
{
    Uint32 pixel_formats[] = {
        SDL_PIXELFORMAT_INDEX8,
        SDL_PIXELFORMAT_RGB332,
        SDL_PIXELFORMAT_RGB444,
        SDL_PIXELFORMAT_BGR444,
        SDL_PIXELFORMAT_RGB555,
        SDL_PIXELFORMAT_BGR555,
        SDL_PIXELFORMAT_ARGB4444,
        SDL_PIXELFORMAT_RGBA4444,
        SDL_PIXELFORMAT_ABGR4444,
        SDL_PIXELFORMAT_BGRA4444,
        SDL_PIXELFORMAT_ARGB1555,
        SDL_PIXELFORMAT_RGBA5551,
        SDL_PIXELFORMAT_ABGR1555,
        SDL_PIXELFORMAT_BGRA5551,
        SDL_PIXELFORMAT_RGB565,
        SDL_PIXELFORMAT_BGR565,
        SDL_PIXELFORMAT_RGB24,
        SDL_PIXELFORMAT_BGR24,
        SDL_PIXELFORMAT_RGB888,
        SDL_PIXELFORMAT_RGBX8888,
        SDL_PIXELFORMAT_BGR888,
        SDL_PIXELFORMAT_BGRX8888,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_PIXELFORMAT_BGRA8888,
        SDL_PIXELFORMAT_ARGB2101010,
    };
    SDL_Surface *face = NULL, *cvt1, *cvt2, *final;
    SDL_PixelFormat *fmt1, *fmt2;
    int i, j, ret = 0;

    /* Create sample surface */
    face = SDLTest_ImageFace();
    SDLTest_AssertCheck(face != NULL, "Verify face surface is not NULL");
    if (face == NULL)
        return TEST_ABORTED;

    /* Set transparent pixel as the pixel at (0,0) */
    if (face->format->palette) {
       ret = SDL_SetColorKey(face, SDL_RLEACCEL, *(Uint8 *) face->pixels);
       SDLTest_AssertPass("Call to SDL_SetColorKey()");
       SDLTest_AssertCheck(ret == 0, "Verify result from SDL_SetColorKey, expected: 0, got: %i", ret);
    }

    for ( i = 0; i < SDL_arraysize(pixel_formats); ++i ) {
        for ( j = 0; j < SDL_arraysize(pixel_formats); ++j ) {
            fmt1 = SDL_AllocFormat(pixel_formats[i]);
            SDL_assert(fmt1 != NULL);
            cvt1 = SDL_ConvertSurface(face, fmt1, 0);
            SDL_assert(cvt1 != NULL);

            fmt2 = SDL_AllocFormat(pixel_formats[j]);
            SDL_assert(fmt1 != NULL);
            cvt2 = SDL_ConvertSurface(cvt1, fmt2, 0);
            SDL_assert(cvt2 != NULL);

            if ( fmt1->BytesPerPixel == face->format->BytesPerPixel &&
                 fmt2->BytesPerPixel == face->format->BytesPerPixel &&
                 (fmt1->Amask != 0) == (face->format->Amask != 0) &&
                 (fmt2->Amask != 0) == (face->format->Amask != 0) ) {
                final = SDL_ConvertSurface( cvt2, face->format, 0 );
                SDL_assert(final != NULL);

                /* Compare surface. */
                ret = SDLTest_CompareSurfaces( face, final, 0 );
                SDLTest_AssertCheck(ret == 0, "Validate result from SDLTest_CompareSurfaces, expected: 0, got: %i", ret);
                SDL_FreeSurface(final);
            }

            SDL_FreeSurface(cvt1);
            SDL_FreeFormat(fmt1);
            SDL_FreeSurface(cvt2);
            SDL_FreeFormat(fmt2);
        }
    }

    /* Clean up. */
    SDL_FreeSurface( face );

    return TEST_COMPLETED;
}


/**
 * @brief Tests sprite loading. A failure case.
 */
int
surface_testLoadFailure(void *arg)
{
    SDL_Surface *face = SDL_LoadBMP("nonexistant.bmp");
    SDLTest_AssertCheck(face == NULL, "SDL_CreateLoadBmp");

    return TEST_COMPLETED;
}

/**
 * @brief Tests some blitting routines.
 */
int
surface_testBlit(void *arg)
{
   int ret;
   SDL_Surface *compareSurface;

   /* Basic blitting */
   _testBlitBlendMode(-1);

   /* Verify result by comparing surfaces */
   compareSurface = SDLTest_ImageBlit();
   ret = SDLTest_CompareSurfaces( testSurface, compareSurface, 0 );
   SDLTest_AssertCheck(ret == 0, "Validate result from SDLTest_CompareSurfaces, expected: 0, got: %i", ret);

   /* Clean up. */
   SDL_FreeSurface(compareSurface);

   return TEST_COMPLETED;
}

/**
 * @brief Tests some blitting routines with color mod
 */
int
surface_testBlitColorMod(void *arg)
{
   int ret;
   SDL_Surface *compareSurface;

   /* Basic blitting with color mod */
   _testBlitBlendMode(-2);

   /* Verify result by comparing surfaces */
   compareSurface = SDLTest_ImageBlitColor();
   ret = SDLTest_CompareSurfaces( testSurface, compareSurface, 0 );
   SDLTest_AssertCheck(ret == 0, "Validate result from SDLTest_CompareSurfaces, expected: 0, got: %i", ret);

   /* Clean up. */
   SDL_FreeSurface(compareSurface);

   return TEST_COMPLETED;
}

/**
 * @brief Tests some blitting routines with alpha mod
 */
int
surface_testBlitAlphaMod(void *arg)
{
   int ret;
   SDL_Surface *compareSurface;

   /* Basic blitting with alpha mod */
   _testBlitBlendMode(-3);

   /* Verify result by comparing surfaces */
   compareSurface = SDLTest_ImageBlitAlpha();
   ret = SDLTest_CompareSurfaces( testSurface, compareSurface, 0 );
   SDLTest_AssertCheck(ret == 0, "Validate result from SDLTest_CompareSurfaces, expected: 0, got: %i", ret);

   /* Clean up. */
   SDL_FreeSurface(compareSurface);

   return TEST_COMPLETED;
}


/**
 * @brief Tests some more blitting routines.
 */
int
surface_testBlitBlendNone(void *arg)
{
   int ret;
   SDL_Surface *compareSurface;

   /* Basic blitting */
   _testBlitBlendMode(SDL_BLENDMODE_NONE);

   /* Verify result by comparing surfaces */
   compareSurface = SDLTest_ImageBlitBlendNone();
   ret = SDLTest_CompareSurfaces( testSurface, compareSurface, 0 );
   SDLTest_AssertCheck(ret == 0, "Validate result from SDLTest_CompareSurfaces, expected: 0, got: %i", ret);

   /* Clean up. */
   SDL_FreeSurface(compareSurface);

   return TEST_COMPLETED;
}

/**
 * @brief Tests some more blitting routines.
 */
int
surface_testBlitBlendBlend(void *arg)
{
   int ret;
   SDL_Surface *compareSurface;

   /* Blend blitting */
   _testBlitBlendMode(SDL_BLENDMODE_BLEND);

   /* Verify result by comparing surfaces */
   compareSurface = SDLTest_ImageBlitBlend();
   ret = SDLTest_CompareSurfaces( testSurface, compareSurface, 0 );
   SDLTest_AssertCheck(ret == 0, "Validate result from SDLTest_CompareSurfaces, expected: 0, got: %i", ret);

   /* Clean up. */
   SDL_FreeSurface(compareSurface);

   return TEST_COMPLETED;
}

/**
 * @brief Tests some more blitting routines.
 */
int
surface_testBlitBlendAdd(void *arg)
{
   int ret;
   SDL_Surface *compareSurface;

   /* Add blitting */
   _testBlitBlendMode(SDL_BLENDMODE_ADD);

   /* Verify result by comparing surfaces */
   compareSurface = SDLTest_ImageBlitBlendAdd();
   ret = SDLTest_CompareSurfaces( testSurface, compareSurface, 0 );
   SDLTest_AssertCheck(ret == 0, "Validate result from SDLTest_CompareSurfaces, expected: 0, got: %i", ret);

   /* Clean up. */
   SDL_FreeSurface(compareSurface);

   return TEST_COMPLETED;
}

/**
 * @brief Tests some more blitting routines.
 */
int
surface_testBlitBlendMod(void *arg)
{
   int ret;
   SDL_Surface *compareSurface;

   /* Mod blitting */
   _testBlitBlendMode(SDL_BLENDMODE_MOD);

   /* Verify result by comparing surfaces */
   compareSurface = SDLTest_ImageBlitBlendMod();
   ret = SDLTest_CompareSurfaces( testSurface, compareSurface, 0 );
   SDLTest_AssertCheck(ret == 0, "Validate result from SDLTest_CompareSurfaces, expected: 0, got: %i", ret);

   /* Clean up. */
   SDL_FreeSurface(compareSurface);

   return TEST_COMPLETED;
}

/**
 * @brief Tests some more blitting routines with loop
 */
int
surface_testBlitBlendLoop(void *arg) {

   int ret;
   SDL_Surface *compareSurface;

   /* All blitting modes */
   _testBlitBlendMode(-4);

   /* Verify result by comparing surfaces */
   compareSurface = SDLTest_ImageBlitBlendAll();
   ret = SDLTest_CompareSurfaces( testSurface, compareSurface, 0 );
   SDLTest_AssertCheck(ret == 0, "Validate result from SDLTest_CompareSurfaces, expected: 0, got: %i", ret);

   /* Clean up. */
   SDL_FreeSurface(compareSurface);

   return TEST_COMPLETED;

}

/* ================= Test References ================== */

/* Surface test cases */
static const SDLTest_TestCaseReference surfaceTest1 =
        { (SDLTest_TestCaseFp)surface_testSaveLoadBitmap, "surface_testSaveLoadBitmap", "Tests sprite saving and loading.", TEST_ENABLED};

static const SDLTest_TestCaseReference surfaceTest2 =
        { (SDLTest_TestCaseFp)surface_testBlit, "surface_testBlit", "Tests basic blitting.", TEST_ENABLED};

static const SDLTest_TestCaseReference surfaceTest3 =
        { (SDLTest_TestCaseFp)surface_testBlitBlendNone, "surface_testBlitBlendNone", "Tests blitting routines with none blending mode.", TEST_ENABLED};

static const SDLTest_TestCaseReference surfaceTest4 =
        { (SDLTest_TestCaseFp)surface_testLoadFailure, "surface_testLoadFailure", "Tests sprite loading. A failure case.", TEST_ENABLED};

static const SDLTest_TestCaseReference surfaceTest5 =
        { (SDLTest_TestCaseFp)surface_testSurfaceConversion, "surface_testSurfaceConversion", "Tests surface conversion.", TEST_ENABLED};

static const SDLTest_TestCaseReference surfaceTest6 =
        { (SDLTest_TestCaseFp)surface_testCompleteSurfaceConversion, "surface_testCompleteSurfaceConversion", "Tests surface conversion across all pixel formats", TEST_ENABLED};

static const SDLTest_TestCaseReference surfaceTest7 =
        { (SDLTest_TestCaseFp)surface_testBlitColorMod, "surface_testBlitColorMod", "Tests some blitting routines with color mod.", TEST_ENABLED};

static const SDLTest_TestCaseReference surfaceTest8 =
        { (SDLTest_TestCaseFp)surface_testBlitAlphaMod, "surface_testBlitAlphaMod", "Tests some blitting routines with alpha mod.", TEST_ENABLED};

/* TODO: rewrite test case, define new test data and re-enable; current implementation fails */
static const SDLTest_TestCaseReference surfaceTest9 =
        { (SDLTest_TestCaseFp)surface_testBlitBlendLoop, "surface_testBlitBlendLoop", "Test blitting routines with various blending modes", TEST_DISABLED};

/* TODO: rewrite test case, define new test data and re-enable; current implementation fails */
static const SDLTest_TestCaseReference surfaceTest10 =
        { (SDLTest_TestCaseFp)surface_testBlitBlendBlend, "surface_testBlitBlendBlend", "Tests blitting routines with blend blending mode.", TEST_DISABLED};

/* TODO: rewrite test case, define new test data and re-enable; current implementation fails */
static const SDLTest_TestCaseReference surfaceTest11 =
        { (SDLTest_TestCaseFp)surface_testBlitBlendAdd, "surface_testBlitBlendAdd", "Tests blitting routines with add blending mode.", TEST_DISABLED};

static const SDLTest_TestCaseReference surfaceTest12 =
        { (SDLTest_TestCaseFp)surface_testBlitBlendMod, "surface_testBlitBlendMod", "Tests blitting routines with mod blending mode.", TEST_ENABLED};

/* Sequence of Surface test cases */
static const SDLTest_TestCaseReference *surfaceTests[] =  {
    &surfaceTest1, &surfaceTest2, &surfaceTest3, &surfaceTest4, &surfaceTest5,
    &surfaceTest6, &surfaceTest7, &surfaceTest8, &surfaceTest9, &surfaceTest10,
    &surfaceTest11, &surfaceTest12, NULL
};

/* Surface test suite (global) */
SDLTest_TestSuiteReference surfaceTestSuite = {
    "Surface",
    _surfaceSetUp,
    surfaceTests,
    _surfaceTearDown

};
