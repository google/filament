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
#define TRUE ((BOOL) 1)
#endif
#ifndef FALSE
#define FALSE ((BOOL) 0)
#endif

#include "png.h"

/* function prototypes */

int main (int argc, char *argv[]);
void usage ();
BOOL png2pnm (FILE *png_file, FILE *pnm_file, FILE *alpha_file,
              BOOL raw, BOOL alpha);
BOOL do_png2pnm (png_struct *png_ptr, png_info *info_ptr,
                 FILE *pnm_file, FILE *alpha_file,
                 BOOL raw, BOOL alpha);

/*
 *  main
 */

int main (int argc, char *argv[])
{
  FILE *fp_rd = stdin;
  FILE *fp_wr = stdout;
  FILE *fp_al = NULL;
  const char *fname_wr = NULL;
  const char *fname_al = NULL;
  BOOL raw = TRUE;
  BOOL alpha = FALSE;
  int argi;
  int ret;

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
            fname_al = argv[argi];
            fprintf (stderr, "PNG2PNM\n");
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
      fname_wr = argv[argi];
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
  ret = png2pnm (fp_rd, fp_wr, fp_al, raw, alpha);

  /* close input file */
  fclose (fp_rd);
  /* close output file */
  fclose (fp_wr);
  /* close alpha file */
  if (alpha)
    fclose (fp_al);

  if (!ret)
  {
    fprintf (stderr, "PNG2PNM\n");
    fprintf (stderr, "Error:  unsuccessful conversion of PNG-image\n");
    if (fname_wr)
      remove (fname_wr); /* no broken output file shall remain behind */
    if (fname_al)
      remove (fname_al); /* ditto */
    exit (1);
  }

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
  png_struct    *png_ptr;
  png_info      *info_ptr;
  BOOL          ret;

  /* initialize the libpng context for reading from png_file */

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
    return FALSE; /* generic libpng error */
  }

  png_init_io (png_ptr, png_file);

  /* do the actual conversion */
  ret = do_png2pnm (png_ptr, info_ptr, pnm_file, alpha_file, raw, alpha);

  /* clean up the libpng structures and their internally-managed data */
  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);

  return ret;
}

/*
 *  do_png2pnm - does the conversion in a fully-initialized libpng context
 */

BOOL do_png2pnm (png_struct *png_ptr, png_info *info_ptr,
                 FILE *pnm_file, FILE *alpha_file,
                 BOOL raw, BOOL alpha)
{
  png_byte      **row_pointers;
  png_byte      *pix_ptr;
  png_uint_32   width;
  png_uint_32   height;
  int           bit_depth;
  int           channels;
  int           color_type;
  int           alpha_present;
  png_uint_32   row, col, i;
  long          dep_16;

  /* set up the image transformations that are necessary for the PNM format */

  /* set up (if applicable) the expansion of paletted images to full-color rgb,
   * and the expansion of transparency maps to full alpha-channel */
  png_set_expand (png_ptr);

  /* set up (if applicable) the expansion of grayscale images to bit-depth 8 */
  png_set_expand_gray_1_2_4_to_8 (png_ptr);

  /* read the image file, with all of the above image transforms applied */
  png_read_png (png_ptr, info_ptr, 0, NULL);

  /* get the image size, bit-depth and color-type */
  png_get_IHDR (png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
                NULL, NULL, NULL);

  /* calculate the number of channels and store alpha-presence */
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
    fprintf (stderr, "Warning:  no alpha channel in PNG file\n");
    return FALSE;
  }

  /* get address of internally-allocated image data */
  row_pointers = png_get_rows (png_ptr, info_ptr);

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

  for (row = 0; row < height; row++)
  {
    pix_ptr = row_pointers[row];
    for (col = 0; col < width; col++)
    {
      for (i = 0; i < (png_uint_32) (channels - alpha_present); i++)
      {
        if (raw)
        {
          fputc ((int) *pix_ptr++, pnm_file);
          if (bit_depth == 16)
            fputc ((int) *pix_ptr++, pnm_file);
        }
        else
        {
          if (bit_depth == 16)
          {
            dep_16 = ((long) *pix_ptr++) << 8;
            dep_16 += ((long) *pix_ptr++);
            fprintf (pnm_file, "%ld ", dep_16);
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
          /* skip the alpha-channel */
          pix_ptr++;
          if (bit_depth == 16)
            pix_ptr++;
        }
        else
        {
          /* output the alpha-channel as pgm file */
          if (raw)
          {
            fputc ((int) *pix_ptr++, alpha_file);
            if (bit_depth == 16)
              fputc ((int) *pix_ptr++, alpha_file);
          }
          else
          {
            if (bit_depth == 16)
            {
              dep_16 = ((long) *pix_ptr++) << 8;
              dep_16 += ((long) *pix_ptr++);
              fprintf (alpha_file, "%ld ", dep_16);
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

  return TRUE;
} /* end of source */
