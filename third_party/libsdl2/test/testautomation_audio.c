/**
 * Original code: automated SDL audio test written by Edgar Simo "bobbens"
 * New/updated tests: aschiffler at ferzkopp dot net
 */

/* quiet windows compiler warnings */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>

#include "SDL.h"
#include "SDL_test.h"

/* ================= Test Case Implementation ================== */

/* Fixture */

void
_audioSetUp(void *arg)
{
    /* Start SDL audio subsystem */
    int ret = SDL_InitSubSystem( SDL_INIT_AUDIO );
        SDLTest_AssertPass("Call to SDL_InitSubSystem(SDL_INIT_AUDIO)");
    SDLTest_AssertCheck(ret==0, "Check result from SDL_InitSubSystem(SDL_INIT_AUDIO)");
    if (ret != 0) {
           SDLTest_LogError("%s", SDL_GetError());
        }
}

void
_audioTearDown(void *arg)
{
    /* Remove a possibly created file from SDL disk writer audio driver; ignore errors */
    remove("sdlaudio.raw");

    SDLTest_AssertPass("Cleanup of test files completed");
}


/* Global counter for callback invocation */
int _audio_testCallbackCounter;

/* Global accumulator for total callback length */
int _audio_testCallbackLength;


/* Test callback function */
void SDLCALL _audio_testCallback(void *userdata, Uint8 *stream, int len)
{
   /* track that callback was called */
   _audio_testCallbackCounter++;
   _audio_testCallbackLength += len;
}


/* Test case functions */

/**
 * \brief Stop and restart audio subsystem
 *
 * \sa https://wiki.libsdl.org/SDL_QuitSubSystem
 * \sa https://wiki.libsdl.org/SDL_InitSubSystem
 */
int audio_quitInitAudioSubSystem()
{
    /* Stop SDL audio subsystem */
    SDL_QuitSubSystem( SDL_INIT_AUDIO );
        SDLTest_AssertPass("Call to SDL_QuitSubSystem(SDL_INIT_AUDIO)");

        /* Restart audio again */
        _audioSetUp(NULL);

    return TEST_COMPLETED;
}

/**
 * \brief Start and stop audio directly
 *
 * \sa https://wiki.libsdl.org/SDL_InitAudio
 * \sa https://wiki.libsdl.org/SDL_QuitAudio
 */
int audio_initQuitAudio()
{
        int result;
    int i, iMax;
    const char* audioDriver;

    /* Stop SDL audio subsystem */
    SDL_QuitSubSystem( SDL_INIT_AUDIO );
        SDLTest_AssertPass("Call to SDL_QuitSubSystem(SDL_INIT_AUDIO)");

        /* Loop over all available audio drivers */
        iMax = SDL_GetNumAudioDrivers();
        SDLTest_AssertPass("Call to SDL_GetNumAudioDrivers()");
        SDLTest_AssertCheck(iMax > 0, "Validate number of audio drivers; expected: >0 got: %d", iMax);
        for (i = 0; i < iMax; i++) {
            audioDriver = SDL_GetAudioDriver(i);
            SDLTest_AssertPass("Call to SDL_GetAudioDriver(%d)", i);
            SDLTest_AssertCheck(audioDriver != NULL, "Audio driver name is not NULL");
            SDLTest_AssertCheck(audioDriver[0] != '\0', "Audio driver name is not empty; got: %s", audioDriver);

            /* Call Init */
            result = SDL_AudioInit(audioDriver);
            SDLTest_AssertPass("Call to SDL_AudioInit('%s')", audioDriver);
            SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0 got: %d", result);

            /* Call Quit */
            SDL_AudioQuit();
            SDLTest_AssertPass("Call to SDL_AudioQuit()");
    }

    /* NULL driver specification */
    audioDriver = NULL;

    /* Call Init */
    result = SDL_AudioInit(audioDriver);
    SDLTest_AssertPass("Call to SDL_AudioInit(NULL)");
    SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0 got: %d", result);

    /* Call Quit */
    SDL_AudioQuit();
    SDLTest_AssertPass("Call to SDL_AudioQuit()");

        /* Restart audio again */
        _audioSetUp(NULL);

    return TEST_COMPLETED;
}

/**
 * \brief Start, open, close and stop audio
 *
 * \sa https://wiki.libsdl.org/SDL_InitAudio
 * \sa https://wiki.libsdl.org/SDL_OpenAudio
 * \sa https://wiki.libsdl.org/SDL_CloseAudio
 * \sa https://wiki.libsdl.org/SDL_QuitAudio
 */
int audio_initOpenCloseQuitAudio()
{
    int result, expectedResult;
    int i, iMax, j, k;
    const char* audioDriver;
    SDL_AudioSpec desired;

    /* Stop SDL audio subsystem */
    SDL_QuitSubSystem( SDL_INIT_AUDIO );
        SDLTest_AssertPass("Call to SDL_QuitSubSystem(SDL_INIT_AUDIO)");

        /* Loop over all available audio drivers */
        iMax = SDL_GetNumAudioDrivers();
        SDLTest_AssertPass("Call to SDL_GetNumAudioDrivers()");
        SDLTest_AssertCheck(iMax > 0, "Validate number of audio drivers; expected: >0 got: %d", iMax);
        for (i = 0; i < iMax; i++) {
            audioDriver = SDL_GetAudioDriver(i);
            SDLTest_AssertPass("Call to SDL_GetAudioDriver(%d)", i);
            SDLTest_AssertCheck(audioDriver != NULL, "Audio driver name is not NULL");
            SDLTest_AssertCheck(audioDriver[0] != '\0', "Audio driver name is not empty; got: %s", audioDriver);

            /* Change specs */
            for (j = 0; j < 2; j++) {

                /* Call Init */
                result = SDL_AudioInit(audioDriver);
                SDLTest_AssertPass("Call to SDL_AudioInit('%s')", audioDriver);
                SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0 got: %d", result);

                /* Set spec */
                SDL_memset(&desired, 0, sizeof(desired));
                switch (j) {
                    case 0:
                    /* Set standard desired spec */
                    desired.freq = 22050;
                    desired.format = AUDIO_S16SYS;
                    desired.channels = 2;
                    desired.samples = 4096;
                    desired.callback = _audio_testCallback;
                    desired.userdata = NULL;

                    case 1:
                    /* Set custom desired spec */
                    desired.freq = 48000;
                    desired.format = AUDIO_F32SYS;
                    desired.channels = 2;
                    desired.samples = 2048;
                    desired.callback = _audio_testCallback;
                    desired.userdata = NULL;
                    break;
            }

            /* Call Open (maybe multiple times) */
            for (k=0; k <= j; k++) {
                result = SDL_OpenAudio(&desired, NULL);
                SDLTest_AssertPass("Call to SDL_OpenAudio(desired_spec_%d, NULL), call %d", j, k+1);
                expectedResult = (k==0) ? 0 : -1;
                SDLTest_AssertCheck(result == expectedResult, "Verify return value; expected: %d, got: %d", expectedResult, result);
            }

            /* Call Close (maybe multiple times) */
            for (k=0; k <= j; k++) {
                SDL_CloseAudio();
                SDLTest_AssertPass("Call to SDL_CloseAudio(), call %d", k+1);
            }

            /* Call Quit (maybe multiple times) */
            for (k=0; k <= j; k++) {
                SDL_AudioQuit();
                SDLTest_AssertPass("Call to SDL_AudioQuit(), call %d", k+1);
            }

        } /* spec loop */
    } /* driver loop */

        /* Restart audio again */
        _audioSetUp(NULL);

    return TEST_COMPLETED;
}

/**
 * \brief Pause and unpause audio
 *
 * \sa https://wiki.libsdl.org/SDL_PauseAudio
 */
int audio_pauseUnpauseAudio()
{
    int result;
    int i, iMax, j, k, l;
    int totalDelay;
    int pause_on;
    int originalCounter;
    const char* audioDriver;
    SDL_AudioSpec desired;

    /* Stop SDL audio subsystem */
    SDL_QuitSubSystem( SDL_INIT_AUDIO );
        SDLTest_AssertPass("Call to SDL_QuitSubSystem(SDL_INIT_AUDIO)");

        /* Loop over all available audio drivers */
        iMax = SDL_GetNumAudioDrivers();
        SDLTest_AssertPass("Call to SDL_GetNumAudioDrivers()");
        SDLTest_AssertCheck(iMax > 0, "Validate number of audio drivers; expected: >0 got: %d", iMax);
        for (i = 0; i < iMax; i++) {
            audioDriver = SDL_GetAudioDriver(i);
            SDLTest_AssertPass("Call to SDL_GetAudioDriver(%d)", i);
            SDLTest_AssertCheck(audioDriver != NULL, "Audio driver name is not NULL");
            SDLTest_AssertCheck(audioDriver[0] != '\0', "Audio driver name is not empty; got: %s", audioDriver);

            /* Change specs */
            for (j = 0; j < 2; j++) {

                /* Call Init */
                result = SDL_AudioInit(audioDriver);
                SDLTest_AssertPass("Call to SDL_AudioInit('%s')", audioDriver);
                SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0 got: %d", result);

                /* Set spec */
                SDL_memset(&desired, 0, sizeof(desired));
                switch (j) {
                    case 0:
                    /* Set standard desired spec */
                    desired.freq = 22050;
                    desired.format = AUDIO_S16SYS;
                    desired.channels = 2;
                    desired.samples = 4096;
                    desired.callback = _audio_testCallback;
                    desired.userdata = NULL;

                    case 1:
                    /* Set custom desired spec */
                    desired.freq = 48000;
                    desired.format = AUDIO_F32SYS;
                    desired.channels = 2;
                    desired.samples = 2048;
                    desired.callback = _audio_testCallback;
                    desired.userdata = NULL;
                    break;
            }

            /* Call Open */
            result = SDL_OpenAudio(&desired, NULL);
            SDLTest_AssertPass("Call to SDL_OpenAudio(desired_spec_%d, NULL)", j);
            SDLTest_AssertCheck(result == 0, "Verify return value; expected: 0 got: %d", result);

            /* Start and stop audio multiple times */
            for (l=0; l<3; l++) {
                SDLTest_Log("Pause/Unpause iteration: %d", l+1);
            
                /* Reset callback counters */
                _audio_testCallbackCounter = 0;
                _audio_testCallbackLength = 0;

                /* Un-pause audio to start playing (maybe multiple times) */
                pause_on = 0;
                for (k=0; k <= j; k++) {
                    SDL_PauseAudio(pause_on);
                    SDLTest_AssertPass("Call to SDL_PauseAudio(%d), call %d", pause_on, k+1);
                }
            
                /* Wait for callback */
                totalDelay = 0;
                do {
                    SDL_Delay(10);
                    totalDelay += 10;
                } 
                while (_audio_testCallbackCounter == 0 && totalDelay < 1000);
                SDLTest_AssertCheck(_audio_testCallbackCounter > 0, "Verify callback counter; expected: >0 got: %d", _audio_testCallbackCounter);
                SDLTest_AssertCheck(_audio_testCallbackLength > 0, "Verify callback length; expected: >0 got: %d", _audio_testCallbackLength);

                /* Pause audio to stop playing (maybe multiple times) */
                for (k=0; k <= j; k++) {
                    pause_on = (k==0) ? 1 : SDLTest_RandomIntegerInRange(99, 9999);
                    SDL_PauseAudio(pause_on);
                    SDLTest_AssertPass("Call to SDL_PauseAudio(%d), call %d", pause_on, k+1);
                }
            
                /* Ensure callback is not called again */
                originalCounter = _audio_testCallbackCounter;
                SDL_Delay(totalDelay + 10);
                SDLTest_AssertCheck(originalCounter == _audio_testCallbackCounter, "Verify callback counter; expected: %d, got: %d", originalCounter, _audio_testCallbackCounter);
            }

            /* Call Close */
            SDL_CloseAudio();
            SDLTest_AssertPass("Call to SDL_CloseAudio()");

            /* Call Quit */
            SDL_AudioQuit();
            SDLTest_AssertPass("Call to SDL_AudioQuit()");

        } /* spec loop */
    } /* driver loop */

    /* Restart audio again */
    _audioSetUp(NULL);

    return TEST_COMPLETED;
}

/**
 * \brief Enumerate and name available audio devices (output and capture).
 *
 * \sa https://wiki.libsdl.org/SDL_GetNumAudioDevices
 * \sa https://wiki.libsdl.org/SDL_GetAudioDeviceName
 */
int audio_enumerateAndNameAudioDevices()
{
   int t, tt;
   int i, n, nn;
   const char *name, *nameAgain;

   /* Iterate over types: t=0 output device, t=1 input/capture device */
   for (t=0; t<2; t++) {

      /* Get number of devices. */
      n = SDL_GetNumAudioDevices(t);
      SDLTest_AssertPass("Call to SDL_GetNumAudioDevices(%i)", t);
      SDLTest_Log("Number of %s devices < 0, reported as %i", (t) ? "capture" : "output", n);
      SDLTest_AssertCheck(n >= 0, "Validate result is >= 0, got: %i", n);

      /* Variation of non-zero type */
      if (t==1) {
         tt = t + SDLTest_RandomIntegerInRange(1,10);
         nn = SDL_GetNumAudioDevices(tt);
         SDLTest_AssertCheck(n==nn, "Verify result from SDL_GetNumAudioDevices(%i), expected same number of audio devices %i, got %i", tt, n, nn);
         nn = SDL_GetNumAudioDevices(-tt);
         SDLTest_AssertCheck(n==nn, "Verify result from SDL_GetNumAudioDevices(%i), expected same number of audio devices %i, got %i", -tt, n, nn);
      }

      /* List devices. */
      if (n>0) {
         for (i=0; i<n; i++) {
            name = SDL_GetAudioDeviceName(i, t);
            SDLTest_AssertPass("Call to SDL_GetAudioDeviceName(%i, %i)", i, t);
            SDLTest_AssertCheck(name != NULL, "Verify result from SDL_GetAudioDeviceName(%i, %i) is not NULL", i, t);
            if (name != NULL) {
              SDLTest_AssertCheck(name[0] != '\0', "verify result from SDL_GetAudioDeviceName(%i, %i) is not empty, got: '%s'", i, t, name);
              if (t==1) {
                  /* Also try non-zero type */
                  tt = t + SDLTest_RandomIntegerInRange(1,10);
                  nameAgain = SDL_GetAudioDeviceName(i, tt);
                  SDLTest_AssertCheck(nameAgain != NULL, "Verify result from SDL_GetAudioDeviceName(%i, %i) is not NULL", i, tt);
                  if (nameAgain != NULL) {
                    SDLTest_AssertCheck(nameAgain[0] != '\0', "Verify result from SDL_GetAudioDeviceName(%i, %i) is not empty, got: '%s'", i, tt, nameAgain);
                    SDLTest_AssertCheck(SDL_strcmp(name, nameAgain)==0,
                      "Verify SDL_GetAudioDeviceName(%i, %i) and SDL_GetAudioDeviceName(%i %i) return the same string",
                      i, t, i, tt);
                  }
               }
            }
         }
      }
   }

   return TEST_COMPLETED;
}

/**
 * \brief Negative tests around enumeration and naming of audio devices.
 *
 * \sa https://wiki.libsdl.org/SDL_GetNumAudioDevices
 * \sa https://wiki.libsdl.org/SDL_GetAudioDeviceName
 */
int audio_enumerateAndNameAudioDevicesNegativeTests()
{
   int t;
   int i, j, no, nc;
   const char *name;

   /* Get number of devices. */
   no = SDL_GetNumAudioDevices(0);
   SDLTest_AssertPass("Call to SDL_GetNumAudioDevices(0)");
   nc = SDL_GetNumAudioDevices(1);
   SDLTest_AssertPass("Call to SDL_GetNumAudioDevices(1)");

   /* Invalid device index when getting name */
   for (t=0; t<2; t++) {
      /* Negative device index */
      i = SDLTest_RandomIntegerInRange(-10,-1);
      name = SDL_GetAudioDeviceName(i, t);
      SDLTest_AssertPass("Call to SDL_GetAudioDeviceName(%i, %i)", i, t);
      SDLTest_AssertCheck(name == NULL, "Check SDL_GetAudioDeviceName(%i, %i) result NULL, expected NULL, got: %s", i, t, (name == NULL) ? "NULL" : name);

      /* Device index past range */
      for (j=0; j<3; j++) {
         i = (t) ? nc+j : no+j;
         name = SDL_GetAudioDeviceName(i, t);
         SDLTest_AssertPass("Call to SDL_GetAudioDeviceName(%i, %i)", i, t);
         SDLTest_AssertCheck(name == NULL, "Check SDL_GetAudioDeviceName(%i, %i) result, expected: NULL, got: %s", i, t, (name == NULL) ? "NULL" : name);
      }

      /* Capture index past capture range but within output range */
      if ((no>0) && (no>nc) && (t==1)) {
         i = no-1;
         name = SDL_GetAudioDeviceName(i, t);
         SDLTest_AssertPass("Call to SDL_GetAudioDeviceName(%i, %i)", i, t);
         SDLTest_AssertCheck(name == NULL, "Check SDL_GetAudioDeviceName(%i, %i) result, expected: NULL, got: %s", i, t, (name == NULL) ? "NULL" : name);
      }
   }

   return TEST_COMPLETED;
}


/**
 * \brief Checks available audio driver names.
 *
 * \sa https://wiki.libsdl.org/SDL_GetNumAudioDrivers
 * \sa https://wiki.libsdl.org/SDL_GetAudioDriver
 */
int audio_printAudioDrivers()
{
   int i, n;
   const char *name;

   /* Get number of drivers */
   n = SDL_GetNumAudioDrivers();
   SDLTest_AssertPass("Call to SDL_GetNumAudioDrivers()");
   SDLTest_AssertCheck(n>=0, "Verify number of audio drivers >= 0, got: %i", n);

   /* List drivers. */
   if (n>0)
   {
      for (i=0; i<n; i++) {
         name = SDL_GetAudioDriver(i);
         SDLTest_AssertPass("Call to SDL_GetAudioDriver(%i)", i);
         SDLTest_AssertCheck(name != NULL, "Verify returned name is not NULL");
         if (name != NULL) {
            SDLTest_AssertCheck(name[0] != '\0', "Verify returned name is not empty, got: '%s'", name);
         }
      }
   }

   return TEST_COMPLETED;
}


/**
 * \brief Checks current audio driver name with initialized audio.
 *
 * \sa https://wiki.libsdl.org/SDL_GetCurrentAudioDriver
 */
int audio_printCurrentAudioDriver()
{
   /* Check current audio driver */
   const char *name = SDL_GetCurrentAudioDriver();
   SDLTest_AssertPass("Call to SDL_GetCurrentAudioDriver()");
   SDLTest_AssertCheck(name != NULL, "Verify returned name is not NULL");
   if (name != NULL) {
      SDLTest_AssertCheck(name[0] != '\0', "Verify returned name is not empty, got: '%s'", name);
   }

   return TEST_COMPLETED;
}

/* Definition of all formats, channels, and frequencies used to test audio conversions */
const int _numAudioFormats = 18;
SDL_AudioFormat _audioFormats[] = { AUDIO_S8, AUDIO_U8, AUDIO_S16LSB, AUDIO_S16MSB, AUDIO_S16SYS, AUDIO_S16, AUDIO_U16LSB,
                AUDIO_U16MSB, AUDIO_U16SYS, AUDIO_U16, AUDIO_S32LSB, AUDIO_S32MSB, AUDIO_S32SYS, AUDIO_S32,
                                AUDIO_F32LSB, AUDIO_F32MSB, AUDIO_F32SYS, AUDIO_F32 };
char *_audioFormatsVerbose[] = { "AUDIO_S8", "AUDIO_U8", "AUDIO_S16LSB", "AUDIO_S16MSB", "AUDIO_S16SYS", "AUDIO_S16", "AUDIO_U16LSB",
                "AUDIO_U16MSB", "AUDIO_U16SYS", "AUDIO_U16", "AUDIO_S32LSB", "AUDIO_S32MSB", "AUDIO_S32SYS", "AUDIO_S32",
                                "AUDIO_F32LSB", "AUDIO_F32MSB", "AUDIO_F32SYS", "AUDIO_F32" };
const int _numAudioChannels = 4;
Uint8 _audioChannels[] = { 1, 2, 4, 6 };
const int _numAudioFrequencies = 4;
int _audioFrequencies[] = { 11025, 22050, 44100, 48000 };


/**
 * \brief Builds various audio conversion structures
 *
 * \sa https://wiki.libsdl.org/SDL_BuildAudioCVT
 */
int audio_buildAudioCVT()
{
  int result;
  SDL_AudioCVT  cvt;
  SDL_AudioSpec spec1;
  SDL_AudioSpec spec2;
  int i, ii, j, jj, k, kk;

  /* No conversion needed */
  spec1.format = AUDIO_S16LSB;
  spec1.channels = 2;
  spec1.freq = 22050;
  result = SDL_BuildAudioCVT(&cvt, spec1.format, spec1.channels, spec1.freq,
                                   spec1.format, spec1.channels, spec1.freq);
  SDLTest_AssertPass("Call to SDL_BuildAudioCVT(spec1 ==> spec1)");
  SDLTest_AssertCheck(result == 0, "Verify result value; expected: 0, got: %i", result);

  /* Typical conversion */
  spec1.format = AUDIO_S8;
  spec1.channels = 1;
  spec1.freq = 22050;
  spec2.format = AUDIO_S16LSB;
  spec2.channels = 2;
  spec2.freq = 44100;
  result = SDL_BuildAudioCVT(&cvt, spec1.format, spec1.channels, spec1.freq,
                                   spec2.format, spec2.channels, spec2.freq);
  SDLTest_AssertPass("Call to SDL_BuildAudioCVT(spec1 ==> spec2)");
  SDLTest_AssertCheck(result == 1, "Verify result value; expected: 1, got: %i", result);

  /* All source conversions with random conversion targets, allow 'null' conversions */
  for (i = 0; i < _numAudioFormats; i++) {
    for (j = 0; j < _numAudioChannels; j++) {
      for (k = 0; k < _numAudioFrequencies; k++) {
        spec1.format = _audioFormats[i];
        spec1.channels = _audioChannels[j];
        spec1.freq = _audioFrequencies[k];
        ii = SDLTest_RandomIntegerInRange(0, _numAudioFormats - 1);
        jj = SDLTest_RandomIntegerInRange(0, _numAudioChannels - 1);
        kk = SDLTest_RandomIntegerInRange(0, _numAudioFrequencies - 1);
        spec2.format = _audioFormats[ii];
        spec2.channels = _audioChannels[jj];
        spec2.freq = _audioFrequencies[kk];
        result = SDL_BuildAudioCVT(&cvt, spec1.format, spec1.channels, spec1.freq,
                                         spec2.format, spec2.channels, spec2.freq);
        SDLTest_AssertPass("Call to SDL_BuildAudioCVT(format[%i]=%s(%i),channels[%i]=%i,freq[%i]=%i ==> format[%i]=%s(%i),channels[%i]=%i,freq[%i]=%i)",
            i, _audioFormatsVerbose[i], spec1.format, j, spec1.channels, k, spec1.freq, ii, _audioFormatsVerbose[ii], spec2.format, jj, spec2.channels, kk, spec2.freq);
        SDLTest_AssertCheck(result == 0 || result == 1, "Verify result value; expected: 0 or 1, got: %i", result);
        if (result<0) {
          SDLTest_LogError("%s", SDL_GetError());
        } else {
          SDLTest_AssertCheck(cvt.len_mult > 0, "Verify that cvt.len_mult value; expected: >0, got: %i", cvt.len_mult);
        }
      }
    }
  }

   return TEST_COMPLETED;
}

/**
 * \brief Checkes calls with invalid input to SDL_BuildAudioCVT
 *
 * \sa https://wiki.libsdl.org/SDL_BuildAudioCVT
 */
int audio_buildAudioCVTNegative()
{
  const char *expectedError = "Parameter 'cvt' is invalid";
  const char *error;
  int result;
  SDL_AudioCVT  cvt;
  SDL_AudioSpec spec1;
  SDL_AudioSpec spec2;
  int i;
  char message[256];

  /* Valid format */
  spec1.format = AUDIO_S8;
  spec1.channels = 1;
  spec1.freq = 22050;
  spec2.format = AUDIO_S16LSB;
  spec2.channels = 2;
  spec2.freq = 44100;

  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");

  /* NULL input for CVT buffer */
  result = SDL_BuildAudioCVT((SDL_AudioCVT *)NULL, spec1.format, spec1.channels, spec1.freq,
                                                   spec2.format, spec2.channels, spec2.freq);
  SDLTest_AssertPass("Call to SDL_BuildAudioCVT(NULL,...)");
  SDLTest_AssertCheck(result == -1, "Verify result value; expected: -1, got: %i", result);
  error = SDL_GetError();
  SDLTest_AssertPass("Call to SDL_GetError()");
  SDLTest_AssertCheck(error != NULL, "Validate that error message was not NULL");
  if (error != NULL) {
      SDLTest_AssertCheck(SDL_strcmp(error, expectedError) == 0,
          "Validate error message, expected: '%s', got: '%s'", expectedError, error);
  }

  /* Invalid conversions */
  for (i = 1; i < 64; i++) {
    /* Valid format to start with */
    spec1.format = AUDIO_S8;
    spec1.channels = 1;
    spec1.freq = 22050;
    spec2.format = AUDIO_S16LSB;
    spec2.channels = 2;
    spec2.freq = 44100;

    SDL_ClearError();
    SDLTest_AssertPass("Call to SDL_ClearError()");

    /* Set various invalid format inputs */
    SDL_strlcpy(message, "Invalid: ", 256);
    if (i & 1) {
        SDL_strlcat(message, " spec1.format", 256);
        spec1.format = 0;
    }
    if (i & 2) {
        SDL_strlcat(message, " spec1.channels", 256);
        spec1.channels = 0;
    }
    if (i & 4) {
        SDL_strlcat(message, " spec1.freq", 256);
        spec1.freq = 0;
    }
    if (i & 8) {
        SDL_strlcat(message, " spec2.format", 256);
        spec2.format = 0;
    }
    if (i & 16) {
        SDL_strlcat(message, " spec2.channels", 256);
        spec2.channels = 0;
    }
    if (i & 32) {
        SDL_strlcat(message, " spec2.freq", 256);
        spec2.freq = 0;
    }
    SDLTest_Log("%s", message);
    result = SDL_BuildAudioCVT(&cvt, spec1.format, spec1.channels, spec1.freq,
                                   spec2.format, spec2.channels, spec2.freq);
    SDLTest_AssertPass("Call to SDL_BuildAudioCVT(spec1 ==> spec2)");
    SDLTest_AssertCheck(result == -1, "Verify result value; expected: -1, got: %i", result);
    error = SDL_GetError();
    SDLTest_AssertPass("Call to SDL_GetError()");
    SDLTest_AssertCheck(error != NULL && error[0] != '\0', "Validate that error message was not NULL or empty");
  }

  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");

  return TEST_COMPLETED;
}

/**
 * \brief Checks current audio status.
 *
 * \sa https://wiki.libsdl.org/SDL_GetAudioStatus
 */
int audio_getAudioStatus()
{
   SDL_AudioStatus result;

   /* Check current audio status */
   result = SDL_GetAudioStatus();
   SDLTest_AssertPass("Call to SDL_GetAudioStatus()");
   SDLTest_AssertCheck(result == SDL_AUDIO_STOPPED || result == SDL_AUDIO_PLAYING || result == SDL_AUDIO_PAUSED,
        "Verify returned value; expected: STOPPED (%i) | PLAYING (%i) | PAUSED (%i), got: %i",
        SDL_AUDIO_STOPPED, SDL_AUDIO_PLAYING, SDL_AUDIO_PAUSED, result);

   return TEST_COMPLETED;
}



/**
 * \brief Opens, checks current audio status, and closes a device.
 *
 * \sa https://wiki.libsdl.org/SDL_GetAudioStatus
 */
int audio_openCloseAndGetAudioStatus()
{
   SDL_AudioStatus result;
   int i;
   int count;
   char *device;
   SDL_AudioDeviceID id;
   SDL_AudioSpec desired, obtained;

   /* Get number of devices. */
   count = SDL_GetNumAudioDevices(0);
   SDLTest_AssertPass("Call to SDL_GetNumAudioDevices(0)");
   if (count > 0) {
     for (i = 0; i < count; i++) {
       /* Get device name */
       device = (char *)SDL_GetAudioDeviceName(i, 0);
       SDLTest_AssertPass("SDL_GetAudioDeviceName(%i,0)", i);
       SDLTest_AssertCheck(device != NULL, "Validate device name is not NULL; got: %s", (device != NULL) ? device : "NULL");
       if (device == NULL) return TEST_ABORTED;

       /* Set standard desired spec */
       desired.freq=22050;
       desired.format=AUDIO_S16SYS;
       desired.channels=2;
       desired.samples=4096;
       desired.callback=_audio_testCallback;
       desired.userdata=NULL;

       /* Open device */
       id = SDL_OpenAudioDevice((const char *)device, 0, &desired, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
       SDLTest_AssertPass("SDL_OpenAudioDevice('%s',...)", device);
       SDLTest_AssertCheck(id > 1, "Validate device ID; expected: >=2, got: %i", id);
       if (id > 1) {

         /* Check device audio status */
         result = SDL_GetAudioDeviceStatus(id);
         SDLTest_AssertPass("Call to SDL_GetAudioDeviceStatus()");
         SDLTest_AssertCheck(result == SDL_AUDIO_STOPPED || result == SDL_AUDIO_PLAYING || result == SDL_AUDIO_PAUSED,
            "Verify returned value; expected: STOPPED (%i) | PLAYING (%i) | PAUSED (%i), got: %i",
            SDL_AUDIO_STOPPED, SDL_AUDIO_PLAYING, SDL_AUDIO_PAUSED, result);

         /* Close device again */
         SDL_CloseAudioDevice(id);
         SDLTest_AssertPass("Call to SDL_CloseAudioDevice()");
       }
     }
   } else {
     SDLTest_Log("No devices to test with");
   }

   return TEST_COMPLETED;
}

/**
 * \brief Locks and unlocks open audio device.
 *
 * \sa https://wiki.libsdl.org/SDL_LockAudioDevice
 * \sa https://wiki.libsdl.org/SDL_UnlockAudioDevice
 */
int audio_lockUnlockOpenAudioDevice()
{
   int i;
   int count;
   char *device;
   SDL_AudioDeviceID id;
   SDL_AudioSpec desired, obtained;

   /* Get number of devices. */
   count = SDL_GetNumAudioDevices(0);
   SDLTest_AssertPass("Call to SDL_GetNumAudioDevices(0)");
   if (count > 0) {
     for (i = 0; i < count; i++) {
       /* Get device name */
       device = (char *)SDL_GetAudioDeviceName(i, 0);
       SDLTest_AssertPass("SDL_GetAudioDeviceName(%i,0)", i);
       SDLTest_AssertCheck(device != NULL, "Validate device name is not NULL; got: %s", (device != NULL) ? device : "NULL");
       if (device == NULL) return TEST_ABORTED;

       /* Set standard desired spec */
       desired.freq=22050;
       desired.format=AUDIO_S16SYS;
       desired.channels=2;
       desired.samples=4096;
       desired.callback=_audio_testCallback;
       desired.userdata=NULL;

       /* Open device */
       id = SDL_OpenAudioDevice((const char *)device, 0, &desired, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
       SDLTest_AssertPass("SDL_OpenAudioDevice('%s',...)", device);
       SDLTest_AssertCheck(id > 1, "Validate device ID; expected: >=2, got: %i", id);
       if (id > 1) {
         /* Lock to protect callback */
         SDL_LockAudioDevice(id);
         SDLTest_AssertPass("SDL_LockAudioDevice(%i)", id);

         /* Simulate callback processing */
         SDL_Delay(10);
         SDLTest_Log("Simulate callback processing - delay");

         /* Unlock again */
         SDL_UnlockAudioDevice(id);
         SDLTest_AssertPass("SDL_UnlockAudioDevice(%i)", id);

         /* Close device again */
         SDL_CloseAudioDevice(id);
         SDLTest_AssertPass("Call to SDL_CloseAudioDevice()");
       }
     }
   } else {
     SDLTest_Log("No devices to test with");
   }

   return TEST_COMPLETED;
}


/**
 * \brief Convert audio using various conversion structures
 *
 * \sa https://wiki.libsdl.org/SDL_BuildAudioCVT
 * \sa https://wiki.libsdl.org/SDL_ConvertAudio
 */
int audio_convertAudio()
{
  int result;
  SDL_AudioCVT  cvt;
  SDL_AudioSpec spec1;
  SDL_AudioSpec spec2;
  int c;
  char message[128];
  int i, ii, j, jj, k, kk, l, ll;

  /* Iterate over bitmask that determines which parameters are modified in the conversion */
  for (c = 1; c < 8; c++) {
    SDL_strlcpy(message, "Changing:", 128);
    if (c & 1) {
      SDL_strlcat(message, " Format", 128);
    }
    if (c & 2) {
      SDL_strlcat(message, " Channels", 128);
    }
    if (c & 4) {
      SDL_strlcat(message, " Frequencies", 128);
    }
    SDLTest_Log("%s", message);
    /* All source conversions with random conversion targets */
    for (i = 0; i < _numAudioFormats; i++) {
      for (j = 0; j < _numAudioChannels; j++) {
        for (k = 0; k < _numAudioFrequencies; k++) {
          spec1.format = _audioFormats[i];
          spec1.channels = _audioChannels[j];
          spec1.freq = _audioFrequencies[k];

          /* Ensure we have a different target format */
          do {
            if (c & 1) {
              ii = SDLTest_RandomIntegerInRange(0, _numAudioFormats - 1);
            } else {
              ii = 1;
            }
            if (c & 2) {
              jj = SDLTest_RandomIntegerInRange(0, _numAudioChannels - 1);
            } else {
              jj= j;
            }
            if (c & 4) {
              kk = SDLTest_RandomIntegerInRange(0, _numAudioFrequencies - 1);
            } else {
              kk = k;
            }
          } while ((i == ii) && (j == jj) && (k == kk));
          spec2.format = _audioFormats[ii];
          spec2.channels = _audioChannels[jj];
          spec2.freq = _audioFrequencies[kk];

          result = SDL_BuildAudioCVT(&cvt, spec1.format, spec1.channels, spec1.freq,
                                           spec2.format, spec2.channels, spec2.freq);
          SDLTest_AssertPass("Call to SDL_BuildAudioCVT(format[%i]=%s(%i),channels[%i]=%i,freq[%i]=%i ==> format[%i]=%s(%i),channels[%i]=%i,freq[%i]=%i)",
            i, _audioFormatsVerbose[i], spec1.format, j, spec1.channels, k, spec1.freq, ii, _audioFormatsVerbose[ii], spec2.format, jj, spec2.channels, kk, spec2.freq);
          SDLTest_AssertCheck(result == 1, "Verify result value; expected: 1, got: %i", result);
          if (result != 1) {
            SDLTest_LogError("%s", SDL_GetError());
          } else {
            SDLTest_AssertCheck(cvt.len_mult > 0, "Verify that cvt.len_mult value; expected: >0, got: %i", cvt.len_mult);
            if (cvt.len_mult < 1) return TEST_ABORTED;

            /* Create some random data to convert */
            l = 64;
            ll = l * cvt.len_mult;
            SDLTest_Log("Creating dummy sample buffer of %i length (%i bytes)", l, ll);
            cvt.len = l;
            cvt.buf = (Uint8 *)SDL_malloc(ll);
            SDLTest_AssertCheck(cvt.buf != NULL, "Check data buffer to convert is not NULL");
            if (cvt.buf == NULL) return TEST_ABORTED;

            /* Convert the data */
            result = SDL_ConvertAudio(&cvt);
            SDLTest_AssertPass("Call to SDL_ConvertAudio()");
            SDLTest_AssertCheck(result == 0, "Verify result value; expected: 0; got: %i", result);
            SDLTest_AssertCheck(cvt.buf != NULL, "Verify conversion buffer is not NULL");
            SDLTest_AssertCheck(cvt.len_ratio > 0.0, "Verify conversion length ratio; expected: >0; got: %f", cvt.len_ratio);

            /* Free converted buffer */
            SDL_free(cvt.buf);
            cvt.buf = NULL;
          }
        }
      }
    }
  }

   return TEST_COMPLETED;
}


/**
 * \brief Opens, checks current connected status, and closes a device.
 *
 * \sa https://wiki.libsdl.org/SDL_AudioDeviceConnected
 */
int audio_openCloseAudioDeviceConnected()
{
   int result = -1;
   int i;
   int count;
   char *device;
   SDL_AudioDeviceID id;
   SDL_AudioSpec desired, obtained;

   /* Get number of devices. */
   count = SDL_GetNumAudioDevices(0);
   SDLTest_AssertPass("Call to SDL_GetNumAudioDevices(0)");
   if (count > 0) {
     for (i = 0; i < count; i++) {
       /* Get device name */
       device = (char *)SDL_GetAudioDeviceName(i, 0);
       SDLTest_AssertPass("SDL_GetAudioDeviceName(%i,0)", i);
       SDLTest_AssertCheck(device != NULL, "Validate device name is not NULL; got: %s", (device != NULL) ? device : "NULL");
       if (device == NULL) return TEST_ABORTED;

       /* Set standard desired spec */
       desired.freq=22050;
       desired.format=AUDIO_S16SYS;
       desired.channels=2;
       desired.samples=4096;
       desired.callback=_audio_testCallback;
       desired.userdata=NULL;

       /* Open device */
       id = SDL_OpenAudioDevice((const char *)device, 0, &desired, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
       SDLTest_AssertPass("SDL_OpenAudioDevice('%s',...)", device);
       SDLTest_AssertCheck(id > 1, "Validate device ID; expected: >1, got: %i", id);
       if (id > 1) {

/* TODO: enable test code when function is available in SDL2 */

#ifdef AUDIODEVICECONNECTED_DEFINED
         /* Get connected status */
         result = SDL_AudioDeviceConnected(id);
         SDLTest_AssertPass("Call to SDL_AudioDeviceConnected()");
#endif
         SDLTest_AssertCheck(result == 1, "Verify returned value; expected: 1; got: %i", result);

         /* Close device again */
         SDL_CloseAudioDevice(id);
         SDLTest_AssertPass("Call to SDL_CloseAudioDevice()");
       }
     }
   } else {
     SDLTest_Log("No devices to test with");
   }

   return TEST_COMPLETED;
}



/* ================= Test Case References ================== */

/* Audio test cases */
static const SDLTest_TestCaseReference audioTest1 =
        { (SDLTest_TestCaseFp)audio_enumerateAndNameAudioDevices, "audio_enumerateAndNameAudioDevices", "Enumerate and name available audio devices (output and capture)", TEST_ENABLED };

static const SDLTest_TestCaseReference audioTest2 =
        { (SDLTest_TestCaseFp)audio_enumerateAndNameAudioDevicesNegativeTests, "audio_enumerateAndNameAudioDevicesNegativeTests", "Negative tests around enumeration and naming of audio devices.", TEST_ENABLED };

static const SDLTest_TestCaseReference audioTest3 =
        { (SDLTest_TestCaseFp)audio_printAudioDrivers, "audio_printAudioDrivers", "Checks available audio driver names.", TEST_ENABLED };

static const SDLTest_TestCaseReference audioTest4 =
        { (SDLTest_TestCaseFp)audio_printCurrentAudioDriver, "audio_printCurrentAudioDriver", "Checks current audio driver name with initialized audio.", TEST_ENABLED };

static const SDLTest_TestCaseReference audioTest5 =
        { (SDLTest_TestCaseFp)audio_buildAudioCVT, "audio_buildAudioCVT", "Builds various audio conversion structures.", TEST_ENABLED };

static const SDLTest_TestCaseReference audioTest6 =
        { (SDLTest_TestCaseFp)audio_buildAudioCVTNegative, "audio_buildAudioCVTNegative", "Checks calls with invalid input to SDL_BuildAudioCVT", TEST_ENABLED };

static const SDLTest_TestCaseReference audioTest7 =
        { (SDLTest_TestCaseFp)audio_getAudioStatus, "audio_getAudioStatus", "Checks current audio status.", TEST_ENABLED };

static const SDLTest_TestCaseReference audioTest8 =
        { (SDLTest_TestCaseFp)audio_openCloseAndGetAudioStatus, "audio_openCloseAndGetAudioStatus", "Opens and closes audio device and get audio status.", TEST_ENABLED };

static const SDLTest_TestCaseReference audioTest9 =
        { (SDLTest_TestCaseFp)audio_lockUnlockOpenAudioDevice, "audio_lockUnlockOpenAudioDevice", "Locks and unlocks an open audio device.", TEST_ENABLED };

/* TODO: enable test when SDL_ConvertAudio segfaults on cygwin have been fixed.    */
/* For debugging, test case can be run manually using --filter audio_convertAudio  */

static const SDLTest_TestCaseReference audioTest10 =
        { (SDLTest_TestCaseFp)audio_convertAudio, "audio_convertAudio", "Convert audio using available formats.", TEST_DISABLED };

/* TODO: enable test when SDL_AudioDeviceConnected has been implemented.           */

static const SDLTest_TestCaseReference audioTest11 =
        { (SDLTest_TestCaseFp)audio_openCloseAudioDeviceConnected, "audio_openCloseAudioDeviceConnected", "Opens and closes audio device and get connected status.", TEST_DISABLED };

static const SDLTest_TestCaseReference audioTest12 =
        { (SDLTest_TestCaseFp)audio_quitInitAudioSubSystem, "audio_quitInitAudioSubSystem", "Quit and re-init audio subsystem.", TEST_ENABLED };

static const SDLTest_TestCaseReference audioTest13 =
        { (SDLTest_TestCaseFp)audio_initQuitAudio, "audio_initQuitAudio", "Init and quit audio drivers directly.", TEST_ENABLED };

static const SDLTest_TestCaseReference audioTest14 =
        { (SDLTest_TestCaseFp)audio_initOpenCloseQuitAudio, "audio_initOpenCloseQuitAudio", "Cycle through init, open, close and quit with various audio specs.", TEST_ENABLED };

static const SDLTest_TestCaseReference audioTest15 =
        { (SDLTest_TestCaseFp)audio_pauseUnpauseAudio, "audio_pauseUnpauseAudio", "Pause and Unpause audio for various audio specs while testing callback.", TEST_ENABLED };

/* Sequence of Audio test cases */
static const SDLTest_TestCaseReference *audioTests[] =  {
    &audioTest1, &audioTest2, &audioTest3, &audioTest4, &audioTest5, &audioTest6,
    &audioTest7, &audioTest8, &audioTest9, &audioTest10, &audioTest11,
    &audioTest12, &audioTest13, &audioTest14, &audioTest15, NULL
};

/* Audio test suite (global) */
SDLTest_TestSuiteReference audioTestSuite = {
    "Audio",
    _audioSetUp,
    audioTests,
    _audioTearDown
};
