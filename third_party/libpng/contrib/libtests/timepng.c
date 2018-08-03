/* timepng.c
 *
 * Copyright (c) 2013 John Cunningham Bowler
 *
 * Last changed in libpng 1.6.1 [March 28, 2013]
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Load an arbitrary number of PNG files (from the command line, or, if there
 * are no arguments on the command line, from stdin) then run a time test by
 * reading each file by row.  The test does nothing with the read result and
 * does no transforms.  The only output is a time as a floating point number of
 * seconds with 9 decimal digits.
 */
#define _POSIX_C_SOURCE 199309L /* for clock_gettime */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <time.h>

#if defined(HAVE_CONFIG_H) && !defined(PNG_NO_CONFIG_H)
#  include <config.h>
#endif

/* Define the following to use this test against your installed libpng, rather
 * than the one being built here:
 */
#ifdef PNG_FREESTANDING_TESTS
#  include <png.h>
#else
#  include "../../png.h"
#endif

static int read_png(FILE *fp)
{
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
   png_infop info_ptr = NULL;
   png_bytep row = NULL, display = NULL;

   if (png_ptr == NULL)
      return 0;

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      if (row != NULL) free(row);
      if (display != NULL) free(display);
      return 0;
   }

   png_init_io(png_ptr, fp);

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
      png_error(png_ptr, "OOM allocating info structure");

   png_read_info(png_ptr, info_ptr);

   {
      png_size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

      row = malloc(rowbytes);
      display = malloc(rowbytes);

      if (row == NULL || display == NULL)
         png_error(png_ptr, "OOM allocating row buffers");

      {
         png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
         int passes = png_set_interlace_handling(png_ptr);
         int pass;

         png_start_read_image(png_ptr);

         for (pass = 0; pass < passes; ++pass)
         {
            png_uint_32 y = height;

            /* NOTE: this trashes the row each time; interlace handling won't
             * work, but this avoids memory thrashing for speed testing.
             */
            while (y-- > 0)
               png_read_row(png_ptr, row, display);
         }
      }
   }

   /* Make sure to read to the end of the file: */
   png_read_end(png_ptr, info_ptr);
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   free(row);
   free(display);
   return 1;
}

static int mytime(struct timespec *t)
{
   /* Do the timing using clock_gettime and the per-process timer. */
   if (!clock_gettime(CLOCK_PROCESS_CPUTIME_ID, t))
      return 1;

   perror("CLOCK_PROCESS_CPUTIME_ID");
   fprintf(stderr, "timepng: could not get the time\n");
   return 0;
}

static int perform_one_test(FILE *fp, int nfiles)
{
   int i;
   struct timespec before, after;

   /* Clear out all errors: */
   rewind(fp);

   if (mytime(&before))
   {
      for (i=0; i<nfiles; ++i)
      {
         if (read_png(fp))
         {
            if (ferror(fp))
            {
               perror("temporary file");
               fprintf(stderr, "file %d: error reading PNG data\n", i);
               return 0;
            }
         }

         else
         {
            perror("temporary file");
            fprintf(stderr, "file %d: error from libpng\n", i);
            return 0;
         }
      }
   }

   else
      return 0;

   if (mytime(&after))
   {
      /* Work out the time difference and print it - this is the only output,
       * so flush it immediately.
       */
      unsigned long s = after.tv_sec - before.tv_sec;
      long ns = after.tv_nsec - before.tv_nsec;

      if (ns < 0)
      {
         --s;
         ns += 1000000000;

         if (ns < 0)
         {
            fprintf(stderr, "timepng: bad clock from kernel\n");
            return 0;
         }
      }

      printf("%lu.%.9ld\n", s, ns);
      fflush(stdout);
      if (ferror(stdout))
      {
         fprintf(stderr, "timepng: error writing output\n");
         return 0;
      }

      /* Successful return */
      return 1;
   }

   else
      return 0;
}

static int add_one_file(FILE *fp, char *name)
{
   FILE *ip = fopen(name, "rb");

   if (ip != NULL)
   {
      int ch;
      for (;;)
      {
         ch = getc(ip);
         if (ch == EOF) break;
         putc(ch, fp);
      }

      if (ferror(ip))
      {
         perror(name);
         fprintf(stderr, "%s: read error\n", name);
         return 0;
      }

      (void)fclose(ip);

      if (ferror(fp))
      {
         perror("temporary file");
         fprintf(stderr, "temporary file write error\n");
         return 0;
      }
   }

   else
   {
      perror(name);
      fprintf(stderr, "%s: open failed\n", name);
      return 0;
   }

   return 1;
}

int main(int argc, char **argv)
{
   int ok = 0;
   FILE *fp = tmpfile();

   if (fp != NULL)
   {
      int err = 0;
      int nfiles = 0;

      if (argc > 1)
      {
         int i;

         for (i=1; i<argc; ++i)
         {
            if (add_one_file(fp, argv[i]))
               ++nfiles;

            else
            {
               err = 1;
               break;
            }
         }
      }

      else
      {
         char filename[FILENAME_MAX+1];

         while (fgets(filename, FILENAME_MAX+1, stdin))
         {
            size_t len = strlen(filename);

            if (filename[len-1] == '\n')
            {
               filename[len-1] = 0;
               if (add_one_file(fp, filename))
                  ++nfiles;

               else
               {
                  err = 1;
                  break;
               }
            }

            else
            {
               fprintf(stderr, "timepng: truncated file name ...%s\n",
                  filename+len-32);
               err = 1;
               break;
            }
         }

         if (ferror(stdin))
         {
            fprintf(stderr, "timepng: stdin: read error\n");
            err = 1;
         }
      }

      if (!err)
      {
         if (nfiles > 0)
            ok = perform_one_test(fp, nfiles);

         else
            fprintf(stderr, "usage: timepng {files} or ls files | timepng\n");
      }

      (void)fclose(fp);
   }

   else
      fprintf(stderr, "timepng: could not open temporary file\n");

   /* Exit code 0 on success. */
   return ok == 0;
}
