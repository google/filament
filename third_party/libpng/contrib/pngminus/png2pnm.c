/*
 *  png2pnm.c --- conversion from PNG-file to PGM/PPM-file
 *  copyright (C) 1999-2019 by Willem van Schaik <willem at schaik dot com>
 *
 *  This software is released under the MIT license. For conditions of
 *  distribution and use, see the LICENSE file part of this package.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#ifndef BOOL
#define BOOL unsigned char
#endif
#ifndef TRUE
#define TRUE (BOOL) 1
#endif
#ifndef FALSE
#define FALSE (BOOL) 0
#endif

/* make png2pnm verbose so we can find problems (needs to be before png.h) */
#ifndef PNG_DEBUG
#define PNG_DEBUG 0
#endif

#include "png.h"

/* function prototypes */

int main (int argc, char *argv[]);
void usage ();
BOOL png2pnm (FILE *png_file, FILE *pnm_file, FILE *alpha_file,
              BOOL raw, BOOL alpha);

/*
 *  main
 */

int main (int argc, char *argv[])
{
  FILE *fp_rd = stdin;
  FILE *fp_wr = stdout;
  FILE *fp_al = NULL;
  BOOL raw = TRUE;
  BOOL alpha = FALSE;
  int argi;

  for (argi = 1; argi < argc; argi++)
  {
    if (argv[argi][0] == '-')
    {
      switch (argv[argi][1])
      {
        case 'n':
          raw = FALSE;
          break;
        case 'r':
          raw = TRUE;
          break;
        case 'a':
          alpha = TRUE;
          argi++;
          if ((fp_al = fopen (argv[argi], "wb")) == NULL)
          {
            fprintf (stderr, "PNM2PNG\n");
            fprintf (stderr, "Error:  cannot create alpha-channel file %s\n",
                     argv[argi]);
            exit (1);
          }
          break;
        case 'h':
        case '?':
          usage ();
          exit (0);
          break;
        default:
          fprintf (stderr, "PNG2PNM\n");
          fprintf (stderr, "Error:  unknown option %s\n", argv[argi]);
          usage ();
          exit (1);
          break;
      } /* end switch */
    }
    else if (fp_rd == stdin)
    {
      if ((fp_rd = fopen (argv[argi], "rb")) == NULL)
      {
        fprintf (stderr, "PNG2PNM\n");
        fprintf (stderr, "Error:  file %s does not exist\n", argv[argi]);
        exit (1);
      }
    }
    else if (fp_wr == stdout)
    {
      if ((fp_wr = fopen (argv[argi], "wb")) == NULL)
      {
        fprintf (stderr, "PNG2PNM\n");
        fprintf (stderr, "Error:  cannot create file %s\n", argv[argi]);
        exit (1);
      }
    }
    else
    {
      fprintf (stderr, "PNG2PNM\n");
      fprintf (stderr, "Error:  too many parameters\n");
      usage ();
      exit (1);
    }
  } /* end for */

#if defined(O_BINARY) && (O_BINARY != 0)
  /* set stdin/stdout if required to binary */
  if (fp_rd == stdin)
    setmode (fileno (stdin), O_BINARY);
  if ((raw) && (fp_wr == stdout))
    setmode (fileno (stdout), O_BINARY);
#endif

  /* call the conversion program itself */
  if (png2pnm (fp_rd, fp_wr, fp_al, raw, alpha) == FALSE)
  {
    fprintf (stderr, "PNG2PNM\n");
    fprintf (stderr, "Error:  unsuccessful conversion of PNG-image\n");
    exit (1);
  }

  /* close input file */
  fclose (fp_rd);
  /* close output file */
  fclose (fp_wr);
  /* close alpha file */
  if (alpha)
    fclose (fp_al);

  return 0;
}

/*
 *  usage
 */

void usage ()
{
  fprintf (stderr, "PNG2PNM\n");
  fprintf (stderr, "   by Willem van Schaik, 1999\n");
  fprintf (stderr, "Usage:  png2pnm [options] <file>.png [<file>.pnm]\n");
  fprintf (stderr, "   or:  ... | png2pnm [options]\n");
  fprintf (stderr, "Options:\n");
  fprintf (stderr,
      "   -r[aw]   write pnm-file in binary format (P4/P5/P6) (default)\n");
  fprintf (stderr, "   -n[oraw] write pnm-file in ascii format (P1/P2/P3)\n");
  fprintf (stderr,
      "   -a[lpha] <file>.pgm write PNG alpha channel as pgm-file\n");
  fprintf (stderr, "   -h | -?  print this help-information\n");
}

/*
 *  png2pnm
 */

BOOL png2pnm (FILE *png_file, FILE *pnm_file, FILE *alpha_file,
              BOOL raw, BOOL alpha)
{
  png_struct    *png_ptr = NULL;
  png_info      *info_ptr = NULL;
  png_byte      buf[8];
  png_byte      *png_pixels = NULL;
  png_byte      **row_pointers = NULL;
  png_byte      *pix_ptr = NULL;
  png_uint_32   row_bytes;

  png_uint_32   width;
  png_uint_32   height;
  int           bit_depth;
  int           channels;
  int           color_type;
  int           alpha_present;
  int           row, col;
  int           ret;
  int           i;
  long          dep_16;

  /* read and check signature in PNG file */
  ret = fread (buf, 1, 8, png_file);
  if (ret != 8)
    return FALSE;

  ret = png_sig_cmp (buf, 0, 8);
  if (ret != 0)
    return FALSE;

  /* create png and info structures */

  png_ptr = png_create_read_struct (png_get_libpng_ver(NULL),
                                    NULL, NULL, NULL);
  if (!png_ptr)
    return FALSE; /* out of memory */

  info_ptr = png_create_info_struct (png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct (&png_ptr, NULL, NULL);
    return FALSE; /* out of memory */
  }

  if (setjmp (png_jmpbuf (png_ptr)))
  {
    png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
    return FALSE;
  }

  /* set up the input control for C streams */
  png_init_io (png_ptr, png_file);
  png_set_sig_bytes (png_ptr, 8); /* we already read the 8 signature bytes */

  /* read the file information */
  png_read_info (png_ptr, info_ptr);

  /* get size and bit-depth of the PNG-image */
  png_get_IHDR (png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
                NULL, NULL, NULL);

  /* set-up the transformations */

  /* transform paletted images into full-color rgb */
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_expand (png_ptr);
  /* expand images to bit-depth 8 (only applicable for grayscale images) */
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand (png_ptr);
  /* transform transparency maps into full alpha-channel */
  if (png_get_valid (png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_expand (png_ptr);

#ifdef NJET
  /* downgrade 16-bit images to 8-bit */
  if (bit_depth == 16)
    png_set_strip_16 (png_ptr);
  /* transform grayscale images into full-color */
  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb (png_ptr);
  /* only if file has a file gamma, we do a correction */
  if (png_get_gAMA (png_ptr, info_ptr, &file_gamma))
    png_set_gamma (png_ptr, (double) 2.2, file_gamma);
#endif

  /* all transformations have been registered; now update info_ptr data,
   * get rowbytes and channels, and allocate image memory */

  png_read_update_info (png_ptr, info_ptr);

  /* get the new color-type and bit-depth (after expansion/stripping) */
  png_get_IHDR (png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
                NULL, NULL, NULL);

  /* check for 16-bit files */
  if (bit_depth == 16)
  {
    raw = FALSE;
#if defined(O_BINARY) && (O_BINARY != 0)
    setmode (fileno (pnm_file), O_BINARY);
#endif
  }

  /* calculate new number of channels and store alpha-presence */
  if (color_type == PNG_COLOR_TYPE_GRAY)
    channels = 1;
  else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    channels = 2;
  else if (color_type == PNG_COLOR_TYPE_RGB)
    channels = 3;
  else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
    channels = 4;
  else
    channels = 0; /* should never happen */
  alpha_present = (channels - 1) % 2;

  /* check if alpha is expected to be present in file */
  if (alpha && !alpha_present)
  {
    fprintf (stderr, "PNG2PNM\n");
    fprintf (stderr, "Error:  PNG-file doesn't contain alpha channel\n");
    exit (1);
  }

  /* row_bytes is the width x number of channels x (bit-depth / 8) */
  row_bytes = png_get_rowbytes (png_ptr, info_ptr);

  if ((row_bytes == 0) ||
      ((size_t) height > (size_t) (-1) / (size_t) row_bytes))
  {
    /* too big */
    png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
    return FALSE;
  }
  if ((png_pixels = (png_byte *)
       malloc ((size_t) row_bytes * (size_t) height)) == NULL)
  {
    png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
    return FALSE;
  }

  if ((row_pointers = (png_byte **)
       malloc ((size_t) height * sizeof (png_byte *))) == NULL)
  {
    png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
    free (png_pixels);
    return FALSE;
  }

  /* set the individual row_pointers to point at the correct offsets */
  for (i = 0; i < ((int) height); i++)
    row_pointers[i] = png_pixels + i * row_bytes;

  /* now we can go ahead and just read the whole image */
  png_read_image (png_ptr, row_pointers);

  /* read rest of file, and get additional chunks in info_ptr - REQUIRED */
  png_read_end (png_ptr, info_ptr);

  /* clean up after the read, and free any memory allocated - REQUIRED */
  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);

  /* write header of PNM file */

  if ((color_type == PNG_COLOR_TYPE_GRAY) ||
      (color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
  {
    fprintf (pnm_file, "%s\n", (raw) ? "P5" : "P2");
    fprintf (pnm_file, "%d %d\n", (int) width, (int) height);
    fprintf (pnm_file, "%ld\n", ((1L << (int) bit_depth) - 1L));
  }
  else if ((color_type == PNG_COLOR_TYPE_RGB) ||
           (color_type == PNG_COLOR_TYPE_RGB_ALPHA))
  {
    fprintf (pnm_file, "%s\n", (raw) ? "P6" : "P3");
    fprintf (pnm_file, "%d %d\n", (int) width, (int) height);
    fprintf (pnm_file, "%ld\n", ((1L << (int) bit_depth) - 1L));
  }

  /* write header of PGM file with alpha channel */

  if ((alpha) &&
      ((color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ||
       (color_type == PNG_COLOR_TYPE_RGB_ALPHA)))
  {
    fprintf (alpha_file, "%s\n", (raw) ? "P5" : "P2");
    fprintf (alpha_file, "%d %d\n", (int) width, (int) height);
    fprintf (alpha_file, "%ld\n", ((1L << (int) bit_depth) - 1L));
  }

  /* write data to PNM file */
  pix_ptr = png_pixels;

  for (row = 0; row < (int) height; row++)
  {
    for (col = 0; col < (int) width; col++)
    {
      for (i = 0; i < (channels - alpha_present); i++)
      {
        if (raw)
        {
          fputc ((int) *pix_ptr++, pnm_file);
        }
        else
        {
          if (bit_depth == 16)
          {
            dep_16 = (long) *pix_ptr++;
            fprintf (pnm_file, "%ld ", (dep_16 << 8) + ((long) *pix_ptr++));
          }
          else
          {
            fprintf (pnm_file, "%ld ", (long) *pix_ptr++);
          }
        }
      }
      if (alpha_present)
      {
        if (!alpha)
        {
          pix_ptr++; /* alpha */
          if (bit_depth == 16)
            pix_ptr++;
        }
        else /* output alpha-channel as pgm file */
        {
          if (raw)
          {
            fputc ((int) *pix_ptr++, alpha_file);
          }
          else
          {
            if (bit_depth == 16)
            {
              dep_16 = (long) *pix_ptr++;
              fprintf (alpha_file, "%ld ", (dep_16 << 8) + (long) *pix_ptr++);
            }
            else
            {
              fprintf (alpha_file, "%ld ", (long) *pix_ptr++);
            }
          }
        }
      } /* end if alpha_present */

      if (!raw)
        if (col % 4 == 3)
          fprintf (pnm_file, "\n");
    } /* end for col */

    if (!raw)
      if (col % 4 != 0)
        fprintf (pnm_file, "\n");
  } /* end for row */

  if (row_pointers != NULL)
    free (row_pointers);
  if (png_pixels != NULL)
    free (png_pixels);

  return TRUE;

} /* end of source */
