/* See LICENSE.txt for the full license governing this code. */
/**
 * \file screenshot.c 
 *
 * Source file for the screenshot API.
 */

#include "SDL_visualtest_mischelper.h"
#include <SDL_test.h>

int
SDLVisualTest_VerifyScreenshots(char* args, char* test_dir, char* verify_dir)
{
    int i, verify_len, return_code, test_len;
    char hash[33];
    char* verify_path; /* path to the bmp file used for verification */
    char* test_path; /* path to the bmp file to be verified */
    SDL_RWops* rw;
    SDL_Surface* verifybmp;

    return_code = 1;

    if(!args)
    {
        SDLTest_LogError("args argument cannot be NULL");
        return_code = -1;
        goto verifyscreenshots_cleanup_generic;
    }
    if(!test_dir)
    {
        SDLTest_LogError("test_dir argument cannot be NULL");
        return_code = -1;
        goto verifyscreenshots_cleanup_generic;
    }
    if(!verify_dir)
    {
        SDLTest_LogError("verify_dir argument cannot be NULL");
        return_code = -1;
        goto verifyscreenshots_cleanup_generic;
    }

    /* generate the MD5 hash */
    SDLVisualTest_HashString(args, hash);

    /* find the verification image */
    /* path_len + hash_len + some number of extra characters */
    verify_len = SDL_strlen(verify_dir) + 32 + 10;
    verify_path = (char*)SDL_malloc(verify_len * sizeof(char));
    if(!verify_path)
    {
        SDLTest_LogError("malloc() failed");
        return_code = -1;
        goto verifyscreenshots_cleanup_generic;
    }
    SDL_snprintf(verify_path, verify_len - 1,
                 "%s/%s.bmp", verify_dir, hash);
    rw = SDL_RWFromFile(verify_path, "rb");
    if(!rw)
    {
        SDLTest_Log("Verification image does not exist."
                    " Please manually verify that the SUT is working correctly.");
        return_code = 2;
        goto verifyscreenshots_cleanup_verifypath;
    }

    /* load the verification image */
    verifybmp = SDL_LoadBMP_RW(rw, 1);
    if(!verifybmp)
    {
        SDLTest_LogError("SDL_LoadBMP_RW() failed");
        return_code = -1;
        goto verifyscreenshots_cleanup_verifypath;
    }

    /* load the test images and compare with the verification image */
    /* path_len + hash_len + some number of extra characters */
    test_len = SDL_strlen(test_dir) + 32 + 10;
    test_path = (char*)SDL_malloc(test_len * sizeof(char));
    if(!test_path)
    {
        SDLTest_LogError("malloc() failed");
        return_code = -1;
        goto verifyscreenshots_cleanup_verifybmp;
    }

    for(i = 1; ; i++)
    {
        SDL_RWops* testrw;
        SDL_Surface* testbmp;

        if(i == 1)
            SDL_snprintf(test_path, test_len - 1, "%s/%s.bmp", test_dir, hash);
        else
            SDL_snprintf(test_path, test_len - 1, "%s/%s_%d.bmp", test_dir, hash, i);
        testrw = SDL_RWFromFile(test_path, "rb");
        
        /* we keep going until we've iterated through the screenshots each
           SUT window */
        if(!testrw)
            break;

        /* load the test screenshot */
        testbmp = SDL_LoadBMP_RW(testrw, 1);
        if(!testbmp)
        {
            SDLTest_LogError("SDL_LoadBMP_RW() failed");
            return_code = -1;
            goto verifyscreenshots_cleanup_verifybmp;
        }

        /* compare with the verification image */
        if(SDLTest_CompareSurfaces(testbmp, verifybmp, 0) != 0)
        {
            return_code = 0;
            SDL_FreeSurface(testbmp);
            goto verifyscreenshots_cleanup_verifybmp;
        }

        SDL_FreeSurface(testbmp);
    }

    if(i == 1)
    {
        SDLTest_LogError("No verification images found");
        return_code = -1;
    }

verifyscreenshots_cleanup_verifybmp:
    SDL_FreeSurface(verifybmp);

verifyscreenshots_cleanup_verifypath:
    SDL_free(verify_path);

verifyscreenshots_cleanup_generic:
    return return_code;
}
