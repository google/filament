/**
 * \file mischelper.c 
 *
 * Source file with miscellaneous helper functions.
 */

#include <SDL_test.h>

void
SDLVisualTest_HashString(char* str, char hash[33])
{
    SDLTest_Md5Context md5c;
    int i;

    if(!str)
    {
        SDLTest_LogError("str argument cannot be NULL");
        return;
    }

    SDLTest_Md5Init(&md5c);
    SDLTest_Md5Update(&md5c, (unsigned char*)str, SDL_strlen(str));
    SDLTest_Md5Final(&md5c);

    /* convert the md5 hash to an array of hexadecimal digits */
    for(i = 0; i < 16; i++)
        SDL_snprintf(hash + 2 * i, 33 - 2 * i, "%02x", (int)md5c.digest[i]);
}