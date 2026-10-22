/*
 *  pnm2png.c --- conversion from PBM/PGM/PPM-file to PNG-file
 *  copyright (C) 1999-2019 by Willem van Schaik <willem at schaik dot com>
 *
 *  This software is released under the MIT license. For conditions of
 *  distribution and use, see the LICENSE file part of this package.
 */

#include <limits.h>
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
BOOL pnm2png (FILE *pnm_file, FILE *png_file, FILE *alpha_file,
              BOOL interlace, BOOL alpha);
BOOL do_pnm2png (png_struct *png_ptr, png_info *info_ptr,
                 FILE *pnm_file, FILE *alpha_file,
                 BOOL interlace, BOOL alpha);
int fscan_pnm_magic (FILE *pnm_file, char *magic_buf, size_t magic_buf_size);
int fscan_pnm_token (FILE *pnm_file, char *token_buf, size_t token_buf_size);
int fscan_pnm_uint_32 (FILE *pnm_file, png_uint_32 *num_ptr);
png_uint_32 get_pnm_data (FILE *pnm_file, int depth);
png_uint_32 get_pnm_value (FILE *pnm_file, int depth);

/*
 *  main
 */

int main (int argc, char *argv[])
{
  FILE *fp_rd = stdin;
  FILE *fp_al = NULL;
  FILE *fp_wr = stdout;
  const char *fname_wr = NULL;
  BOOL interlace = FALSE;
  BOOL alpha = FALSE;
  int argi;
  int ret;

  for (argi = 1; argi < argc; argi++)
  {
    if (argv[argi][0] == '-')
    {
      switch (argv[argi][1])
      {
        case 'i':
          interlace = TRUE;
          break;
        case 'a':
          alpha = TRUE;
          argi++;
          if ((fp_al = fopen (argv[argi], "rb")) == NULL)
          {
            fprintf (stderr, "PNM2PNG\n");
            fprintf (stderr, "Error:  alpha-channel file %s does not exist\n",
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
          fprintf (stderr, "PNM2PNG\n");
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
        fprintf (stderr, "PNM2PNG\n");
        fprintf (stderr, "Error:  file %s does not exist\n", argv[argi]);
        exit (1);
      }
    }
    else if (fp_wr == stdout)
    {
      fname_wr = argv[argi];
      if ((fp_wr = fopen (argv[argi], "wb")) == NULL)
      {
        fprintf (stderr, "PNM2PNG\n");
        fprintf (stderr, "Error:  cannot create PNG-file %s\n", argv[argi]);
        exit (1);
      }
    }
    else
    {
      fprintf (stderr, "PNM2PNG\n");
      fprintf (stderr, "Error:  too many parameters\n");
      usage ();
      exit (1);
    }
  } /* end for */

#if defined(O_BINARY) && (O_BINARY != 0)
  /* set stdin/stdout to binary,
   * we're reading the PNM always! in binary format
   */
  if (fp_rd == stdin)
    setmode (fileno (stdin), O_BINARY);
  if (fp_wr == stdout)
    setmode (fileno (stdout), O_BINARY);
#endif

  /* call the conversion program itself */
  ret = pnm2png (fp_rd, fp_wr, fp_al, interlace, alpha);

  /* close input file */
  fclose (fp_rd);
  /* close output file */
  fclose (fp_wr);
  /* close alpha file */
  if (alpha)
    fclose (fp_al);

  if (!ret)
  {
    fprintf (stderr, "PNM2PNG\n");
    fprintf (stderr, "Error:  unsuccessful converting to PNG-image\n");
    if (fname_wr)
      remove (fname_wr); /* no broken output file shall remain behind */
    exit (1);
  }

  return 0;
}

/*
 *  usage
 */

void usage ()
{
  fprintf (stderr, "PNM2PNG\n");
  fprintf (stderr, "   by Willem van Schaik, 1999\n");
  fprintf (stderr, "Usage:  pnm2png [options] <file>.<pnm> [<file>.png]\n");
  fprintf (stderr, "   or:  ... | pnm2png [options]\n");
  fprintf (stderr, "Options:\n");
  fprintf (stderr, "   -i[nterlace]   write png-file with interlacing on\n");
  fprintf (stderr,
      "   -a[lpha] <file>.pgm read PNG alpha channel as pgm-file\n");
  fprintf (stderr, "   -h | -?  print this help-information\n");
}

/*
 *  pnm2png
 */

BOOL pnm2png (FILE *pnm_file, FILE *png_file, FILE *alpha_file,
              BOOL interlace, BOOL alpha)
{
  png_struct    *png_ptr;
  png_info      *info_ptr;
  BOOL          ret;

  /* initialize the libpng context for writing to png_file */

  png_ptr = png_create_write_struct (png_get_libpng_ver(NULL),
                                     NULL, NULL, NULL);
  if (!png_ptr)
    return FALSE; /* out of memory */

  info_ptr = png_create_info_struct (png_ptr);
  if (!info_ptr)
  {
    png_destroy_write_struct (&png_ptr, NULL);
    return FALSE; /* out of memory */
  }

  if (setjmp (png_jmpbuf (png_ptr)))
  {
    png_destroy_write_struct (&png_ptr, &info_ptr);
    return FALSE; /* generic libpng error */
  }

  png_init_io (png_ptr, png_file);

  /* do the actual conversion */
  ret = do_pnm2png (png_ptr, info_ptr, pnm_file, alpha_file, interlace, alpha);

  /* clean up the libpng structures and their internally-managed data */
  png_destroy_write_struct (&png_ptr, &info_ptr);

  return ret;
}

/*
 *  do_pnm2png - does the conversion in a fully-initialized libpng context
 */

BOOL do_pnm2png (png_struct *png_ptr, png_info *info_ptr,
                 FILE *pnm_file, FILE *alpha_file,
                 BOOL interlace, BOOL alpha)
{
  png_byte      **row_pointers;
  png_byte      *pix_ptr;
  int           bit_depth;
  int           color_type;
  int           channels;
  char          magic_token[4];
  BOOL          raw;
  png_uint_32   width, height, maxval;
  png_uint_32   row_bytes;
  png_uint_32   row, col;
  png_uint_32   val16, i;
  png_uint_32   alpha_width = 0, alpha_height = 0;
  int           alpha_depth = 0, alpha_present = 0;
  BOOL          alpha_raw = FALSE;
  BOOL          packed_bitmap = FALSE;

  /* read header of PNM file */

  if (fscan_pnm_magic (pnm_file, magic_token, sizeof (magic_token)) != 1)
    return FALSE; /* not a PNM file */

  if ((magic_token[1] == '1') || (magic_token[1] == '4'))
  {
    if ((fscan_pnm_uint_32 (pnm_file, &width) != 1) ||
        (fscan_pnm_uint_32 (pnm_file, &height) != 1))
      return FALSE; /* bad PBM file header */
  } else if ((magic_token[1] == '2') || (magic_token[1] == '5') ||
             (magic_token[1] == '3') || (magic_token[1] == '6'))
  {
    if ((fscan_pnm_uint_32 (pnm_file, &width) != 1) ||
        (fscan_pnm_uint_32 (pnm_file, &height) != 1) ||
        (fscan_pnm_uint_32 (pnm_file, &maxval) != 1))
      return FALSE; /* bad PGM/PPM file header */
  }

  if ((magic_token[1] == '1') || (magic_token[1] == '4'))
  {
    raw = (magic_token[1] == '4');
    bit_depth = 1;
    color_type = PNG_COLOR_TYPE_GRAY;
    packed_bitmap = TRUE;
  }
  else if ((magic_token[1] == '2') || (magic_token[1] == '5'))
  {
    raw = (magic_token[1] == '5');
    color_type = PNG_COLOR_TYPE_GRAY;
    if (maxval == 0)
      return FALSE;
    else if (maxval == 1)
      bit_depth = 1;
    else if (maxval <= 3)
      bit_depth = 2;
    else if (maxval <= 15)
      bit_depth = 4;
    else if (maxval <= 255)
      bit_depth = 8;
    else if (maxval <= 65535U)
      bit_depth = 16;
    else /* maxval > 65535U */
      return FALSE;
  }
  else if ((magic_token[1] == '3') || (magic_token[1] == '6'))
  {
    raw = (magic_token[1] == '6');
    color_type = PNG_COLOR_TYPE_RGB;
    if (maxval == 0)
      return FALSE;
    else if (maxval == 1)
      bit_depth = 1;
    else if (maxval <= 3)
      bit_depth = 2;
    else if (maxval <= 15)
      bit_depth = 4;
    else if (maxval <= 255)
      bit_depth = 8;
    else if (maxval <= 65535U)
      bit_depth = 16;
    else /* maxval > 65535U */
      return FALSE;
  }
  else if (magic_token[1] == '7')
  {
    fprintf (stderr, "PNM2PNG can't read PAM (P7) files\n");
    return FALSE;
  }
  else
  {
    return FALSE;
  }

  /* read header of PGM file with alpha channel */

  if (alpha)
  {
    if ((fscan_pnm_magic (alpha_file, magic_token, sizeof (magic_token)) != 1)
        || ((magic_token[1] != '2') && (magic_token[1] != '5')))
      return FALSE; /* not a PGM file */

    if ((fscan_pnm_uint_32 (alpha_file, &alpha_width) != 1) ||
        (fscan_pnm_uint_32 (alpha_file, &alpha_height) != 1) ||
        (fscan_pnm_uint_32 (alpha_file, &maxval) != 1))
      return FALSE; /* bad PGM file header */

    if ((alpha_width != width) || (alpha_height != height))
      return FALSE; /* mismatched PGM dimensions */

    alpha_raw = (magic_token[1] == '5');
    color_type |= PNG_COLOR_MASK_ALPHA;
    if (maxval == 0)
      return FALSE;
    else if (maxval == 1)
      alpha_depth = 1;
    else if (maxval <= 3)
      alpha_depth = 2;
    else if (maxval <= 15)
      alpha_depth = 4;
    else if (maxval <= 255)
      alpha_depth = 8;
    else if (maxval <= 65535U)
      alpha_depth = 16;
    else /* maxval > 65535U */
      return FALSE;
    if (alpha_depth != bit_depth)
      return FALSE;
  } /* end if alpha */

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
    return FALSE; /* NOTREACHED */

  alpha_present = (channels - 1) % 2;

  if (packed_bitmap)
  {
    /* row data is as many bytes as can fit width x channels x bit_depth */
    row_bytes = (width * channels * bit_depth + 7) / 8;
  }
  else
  {
    /* row_bytes is the width x number of channels x (bit-depth / 8) */
    row_bytes = width * channels * ((bit_depth <= 8) ? 1 : 2);
  }

  if ((row_bytes == 0) ||
      ((size_t) height > (size_t) (-1) / (size_t) row_bytes))
  {
    /* too big */
    return FALSE;
  }

  /* allocate the rows using the same memory layout as libpng, and transfer
   * their ownership to libpng, with the responsibility to clean everything up;
   * please note the use of png_calloc instead of png_malloc */
  row_pointers = (png_byte **)
                 png_calloc (png_ptr, height * sizeof (png_byte *));
  png_set_rows (png_ptr, info_ptr, row_pointers);
  png_data_freer (png_ptr, info_ptr, PNG_DESTROY_WILL_FREE_DATA, PNG_FREE_ALL);
  for (row = 0; row < height; row++)
  {
    /* the individual rows should only be allocated after all the previous
     * steps completed successfully, because libpng must handle correctly
     * any image allocation left incomplete after an out-of-memory error */
    row_pointers[row] = (png_byte *) png_malloc (png_ptr, row_bytes);
  }

  /* read the data from PNM file */

  for (row = 0; row < height; row++)
  {
    pix_ptr = row_pointers[row];
    if (packed_bitmap)
    {
      for (i = 0; i < row_bytes; i++)
      {
        /* png supports this format natively so no conversion is needed */
        *pix_ptr++ = get_pnm_data (pnm_file, 8);
      }
    }
    else
    {
      for (col = 0; col < width; col++)
      {
        for (i = 0; i < (png_uint_32) (channels - alpha_present); i++)
        {
          if (raw)
          {
            *pix_ptr++ = get_pnm_data (pnm_file, bit_depth);
            if (bit_depth == 16)
              *pix_ptr++ = get_pnm_data (pnm_file, bit_depth);
          }
          else
          {
            if (bit_depth <= 8)
            {
              *pix_ptr++ = get_pnm_value (pnm_file, bit_depth);
            }
            else
            {
              val16 = get_pnm_value (pnm_file, bit_depth);
              *pix_ptr = (png_byte) ((val16 >> 8) & 0xFF);
              pix_ptr++;
              *pix_ptr = (png_byte) (val16 & 0xFF);
              pix_ptr++;
            }
          }
        }

        if (alpha) /* read alpha-channel from pgm file */
        {
          if (alpha_raw)
          {
            *pix_ptr++ = get_pnm_data (alpha_file, alpha_depth);
            if (alpha_depth == 16)
              *pix_ptr++ = get_pnm_data (alpha_file, alpha_depth);
          }
          else
          {
            if (alpha_depth <= 8)
            {
              *pix_ptr++ = get_pnm_value (alpha_file, bit_depth);
            }
            else
            {
              val16 = get_pnm_value (alpha_file, bit_depth);
              *pix_ptr++ = (png_byte) ((val16 >> 8) & 0xFF);
              *pix_ptr++ = (png_byte) (val16 & 0xFF);
            }
          }
        } /* end if alpha */
      } /* end if packed_bitmap */
    } /* end for col */
  } /* end for row */

  /* we're going to write more or less the same PNG as the input file */
  png_set_IHDR (png_ptr, info_ptr, width, height, bit_depth, color_type,
                (!interlace) ? PNG_INTERLACE_NONE : PNG_INTERLACE_ADAM7,
                PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  if (packed_bitmap == TRUE)
  {
    png_set_packing (png_ptr);
    png_set_invert_mono (png_ptr);
  }

  /* write the file header information */
  png_write_info (png_ptr, info_ptr);

  /* write out the entire image data in one call */
  png_write_image (png_ptr, row_pointers);

  /* write the additional chunks to the PNG file (not really needed) */
  png_write_end (png_ptr, info_ptr);

  return TRUE;
} /* end of pnm2png */

/*
 * fscan_pnm_magic - like fscan_pnm_token below, but expects the magic string
 *                   to start immediately, without any comment or whitespace,
 *                   and to match the regex /^P[1-9]$/
 */

int fscan_pnm_magic (FILE *pnm_file, char *magic_buf, size_t magic_buf_size)
{
  int ret;

  ret = fgetc (pnm_file);
  if (ret == EOF) return 0;
  ungetc (ret, pnm_file);
  if (ret != 'P') return 0;

  /* the string buffer must be at least four bytes long, i.e., the capacity
   * required for strings of at least three characters long, i.e., the minimum
   * required for ensuring that our magic string is exactly two characters long
   */
  if (magic_buf_size < 4) return -1;

  ret = fscan_pnm_token (pnm_file, magic_buf, magic_buf_size);
  if (ret < 1) return ret;

  if ((magic_buf[1] < '1') || (magic_buf[1] > '9')) return 0;
  if (magic_buf[2] != '\0') return 0;

  return 1;
}

/*
 * fscan_pnm_token - extracts the first string token after whitespace,
 *                   and (like fscanf) returns the number of successful
 *                   extractions, which can be either 0 or 1
 */

int fscan_pnm_token (FILE *pnm_file, char *token_buf, size_t token_buf_size)
{
  size_t i = 0;
  int ret;

  /* remove white-space and comment lines */
  do
  {
    ret = fgetc (pnm_file);
    if (ret == '#')
    {
      /* the rest of this line is a comment */
      do
      {
        ret = fgetc (pnm_file);
      }
      while ((ret != '\n') && (ret != '\r') && (ret != EOF));
    }
    if (ret == EOF) break;
    token_buf[i] = (char) ret;
  }
  while ((ret == '\n') || (ret == '\r') || (ret == ' '));

  /* read string */
  do
  {
    ret = fgetc (pnm_file);
    if (ret == EOF) break;
    if (ret == '0')
    {
      /* avoid storing more than one leading '0' in the token buffer,
       * to ensure that all valid (in-range) numeric inputs can fit in. */
      if ((i == 0) && (token_buf[i] == '0')) continue;
    }
    if (++i == token_buf_size - 1) break;
    token_buf[i] = (char) ret;
  }
  while ((ret != '\n') && (ret != '\r') && (ret != ' '));

  token_buf[i] = '\0';
  return (i > 0) ? 1 : 0;
}

/*
 * fscan_pnm_uint_32 - like fscan_token above, but expects the extracted token
 *                     to be numeric, and converts it to an unsigned 32-bit int
 */

int fscan_pnm_uint_32 (FILE *pnm_file, png_uint_32 *num_ptr)
{
  char token[16];
  unsigned long token_value;
  int ret;

  ret = fscan_pnm_token (pnm_file, token, sizeof (token));
  if (ret < 1) return ret;

  if ((token[0] < '0') && (token[0] > '9'))
    return 0; /* the token starts with junk, or a +/- sign, which is invalid */

  ret = sscanf (token, "%lu%*c", &token_value);
  if (ret != 1)
    return 0; /* the token ends with junk */

  *num_ptr = (png_uint_32) token_value;

#if ULONG_MAX > 0xFFFFFFFFUL
  /* saturate the converted number, following the fscanf convention */
  if (token_value > 0xFFFFFFFFUL)
    *num_ptr = 0xFFFFFFFFUL;
#endif

  return 1;
}

/*
 *  get_pnm_data - takes first byte and converts into next pixel value,
 *                 taking as many bits as defined by bit-depth and
 *                 using the bit-depth to fill up a byte (0x0A -> 0xAA)
 */

png_uint_32 get_pnm_data (FILE *pnm_file, int depth)
{
  static int bits_left = 0;
  static int old_value = 0;
  static int mask = 0;
  png_uint_32 ret_value;
  int i;

  if (mask == 0)
    for (i = 0; i < depth; i++)
      mask = (mask >> 1) | 0x80;

  if (bits_left <= 0)
  {
    /* FIXME:
     * signal the premature end of file, instead of pretending to read zeroes
     */
    old_value = fgetc (pnm_file);
    if (old_value == EOF) return 0;
    bits_left = 8;
  }

  ret_value = old_value & mask;
  for (i = 1; i < (8 / depth); i++)
    ret_value = ret_value || (ret_value >> depth);

  old_value = (old_value << depth) & 0xFF;
  bits_left -= depth;

  return ret_value;
}

/*
 *  get_pnm_value - takes first (numeric) string and converts into number,
 *                  using the bit-depth to fill up a byte (0x0A -> 0xAA)
 */

png_uint_32 get_pnm_value (FILE *pnm_file, int depth)
{
  static png_uint_32 mask = 0;
  png_uint_32 ret_value;
  int i;

  if (mask == 0)
    for (i = 0; i < depth; i++)
      mask = (mask << 1) | 0x01;

  if (fscan_pnm_uint_32 (pnm_file, &ret_value) != 1)
  {
    /* FIXME:
     * signal the invalid numeric tokens or the premature end of file,
     * instead of pretending to read zeroes
     */
    return 0;
  }

  ret_value &= mask;

  if (depth < 8)
    for (i = 0; i < (8 / depth); i++)
      ret_value = (ret_value << depth) || ret_value;

  return ret_value;
}

/* end of source */
