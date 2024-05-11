#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <tchar.h>
#include <windows.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

// Uncomment if you want to use system provided zlib.
// #define TINYEXR_USE_MINIZ (0)
// #include <zlib.h>

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

#ifdef __clang__
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif
#endif

#define SIMPLE_API_EXAMPLE
//#define TEST_ZFP_COMPRESSION

#ifdef SIMPLE_API_EXAMPLE

#if 0
static void
SaveAsPFM(const char* filename, int width, int height, float* data)
{
#ifdef _WIN32
  FILE* fp = NULL;
  fopen_s(&fp, filename, "wb");
#else
  FILE* fp = fopen(filename, "wb");
#endif
  if (!fp) {
    fprintf(stderr, "failed to write a PFM file.\n");
    return;
  }

  fprintf(fp, "PF\n");
  fprintf(fp, "%d %d\n", width, height);
  fprintf(fp, "-1\n"); // -1: little endian, 1: big endian

  // RGBA -> RGB
  std::vector<float> rgb(static_cast<size_t>(width*height*3));

  for (size_t i = 0; i < static_cast<size_t>(width * height); i++) {
    rgb[3*i+0] = data[4*i+0];
    rgb[3*i+1] = data[4*i+1];
    rgb[3*i+2] = data[4*i+2];
  }

  fwrite(&rgb.at(0), sizeof(float), static_cast<size_t>(width * height * 3), fp);

  fclose(fp);
}
#endif

#else

static const char* GetPixelType(int id) {
  if (id == TINYEXR_PIXELTYPE_HALF) {
    return "HALF";
  } else if (id == TINYEXR_PIXELTYPE_FLOAT) {
    return "FLOAT";
  } else if (id == TINYEXR_PIXELTYPE_UINT) {
    return "UINT";
  }

  return "???";
}

// Simple tile -> scanline converter. Assumes FLOAT pixel type for all channels.
static void TiledImageToScanlineImage(EXRImage* src, const EXRHeader* header) {
  assert(header->data_window[2] - header->data_window[0] + 1 >= 0);
  assert(header->data_window[3] - header->data_window[1] + 1 >= 0);
  size_t data_width =
      static_cast<size_t>(header->data_window[2] - header->data_window[0] + 1);
  size_t data_height =
      static_cast<size_t>(header->data_window[3] - header->data_window[1] + 1);

  src->images = static_cast<unsigned char**>(
      malloc(sizeof(float*) * static_cast<size_t>(header->num_channels)));
  for (size_t c = 0; c < static_cast<size_t>(header->num_channels); c++) {
    assert(header->pixel_types[c] == TINYEXR_PIXELTYPE_FLOAT);
    src->images[c] = static_cast<unsigned char*>(
        malloc(sizeof(float) * data_width * data_height));
    memset(src->images[c], 0, sizeof(float) * data_width * data_height);
  }

  for (size_t tile_idx = 0; tile_idx < static_cast<size_t>(src->num_tiles);
       tile_idx++) {
    size_t sx = static_cast<size_t>(src->tiles[tile_idx].offset_x *
                                    header->tile_size_x);
    size_t sy = static_cast<size_t>(src->tiles[tile_idx].offset_y *
                                    header->tile_size_y);
    size_t ex = static_cast<size_t>(src->tiles[tile_idx].offset_x *
                                        header->tile_size_x +
                                    src->tiles[tile_idx].width);
    size_t ey = static_cast<size_t>(src->tiles[tile_idx].offset_y *
                                        header->tile_size_y +
                                    src->tiles[tile_idx].height);

    for (size_t c = 0; c < static_cast<size_t>(header->num_channels); c++) {
      float* dst_image = reinterpret_cast<float*>(src->images[c]);
      const float* src_image =
          reinterpret_cast<const float*>(src->tiles[tile_idx].images[c]);
      for (size_t y = 0; y < static_cast<size_t>(ey - sy); y++) {
        for (size_t x = 0; x < static_cast<size_t>(ex - sx); x++) {
          dst_image[(y + sy) * data_width + (x + sx)] =
              src_image[y * static_cast<size_t>(header->tile_size_x) + x];
        }
      }
    }
  }
}
#endif

#if defined(_WIN32)

#if defined(__MINGW32__)
// __wgetmainargs is not defined in windows.h
extern "C" int __wgetmainargs(int*, wchar_t***, wchar_t***, int, int*);
#endif

// https://gist.github.com/trueroad/fb4d0c3f67285bf66804
namespace {
std::vector<char> utf16_to_utf8(const wchar_t* wc) {
  int size = WideCharToMultiByte(CP_UTF8, 0, wc, -1, NULL, 0, NULL, NULL);
  std::vector<char> retval(size);
  if (size) {
    WideCharToMultiByte(CP_UTF8, 0, wc, -1, retval.data(), retval.size(), NULL,
                        NULL);
  } else
    retval.push_back('\0');
  return retval;
}
}  // namespace
#endif

static int test_main(int argc, char** argv);

#if defined(_WIN32)
#if defined(__MINGW32__)
int main() {
  wchar_t** wargv;
  wchar_t** wenpv;
  int argc = 0, si = 0;
  __wgetmainargs(&argc, &wargv, &wenpv, 1, &si);

  std::vector<std::vector<char> > argv_vvc(argc);
  std::vector<char*> argv_vc(argc);

  for (int i = 0; i < argc; i++) {
    argv_vvc.at(i) = utf16_to_utf8(wargv[i]);
    argv_vc.at(i) = argv_vvc.at(i).data();
  }

  // TODO(syoyo): envp

  return test_main(argc, argv_vc.data());
}
#else  // Assume MSVC
int _tmain(int argc, _TCHAR** wargv) {
  std::vector<std::vector<char> > argv_vvc(argc);
  std::vector<char*> argv_vc(argc);

  for (int i = 0; i < argc; i++) {
#if defined(UNICODE) || defined(_UNICODE)
    argv_vvc.at(i) = utf16_to_utf8(wargv[i]);
#else
    size_t slen = _tcslen(wargv[i]);
    std::vector<char> buf(slen + 1);
    memcpy(buf.data(), wargv[i], slen);
    buf[slen] = '\0';
    argv_vvc.at(i) = buf;
#endif

    argv_vc.at(i) = argv_vvc.at(i).data();
  }

  return test_main(argc, argv_vc.data());
}
#endif
#else
int main(int argc, char** argv) { return test_main(argc, argv); }
#endif

int test_main(int argc, char** argv) {
  const char* outfilename = "output_test.exr";
  const char* err = NULL;

  if (argc < 2) {
    fprintf(stderr, "Needs input.exr.\n");
    exit(-1);
  }

  if (argc > 2) {
    outfilename = argv[2];
  }

  const char* input_filename = argv[1];

#ifdef SIMPLE_API_EXAMPLE
  (void)outfilename;
  int width, height;
  float* image;

  int ret = IsEXR(input_filename);
  if (ret != TINYEXR_SUCCESS) {
    fprintf(stderr, "Header err. code %d\n", ret);
    exit(-1);
  }

  ret = LoadEXR(&image, &width, &height, input_filename, &err);
  if (ret != TINYEXR_SUCCESS) {
    if (err) {
      fprintf(stderr, "Load EXR err: %s(code %d)\n", err, ret);
    } else {
      fprintf(stderr, "Load EXR err: code = %d\n", ret);
    }
    FreeEXRErrorMessage(err);
    return ret;
  }
  // SaveAsPFM("output.pfm", width, height, image);
  ret = SaveEXR(image, width, height, 4 /* =RGBA*/,
                1 /* = save as fp16 format */, "output.exr", &err);
  if (ret != TINYEXR_SUCCESS) {
    if (err) {
      fprintf(stderr, "Save EXR err: %s(code %d)\n", err, ret);
    } else {
      fprintf(stderr, "Failed to save EXR image. code = %d\n", ret);
    }
  }
  free(image);

  std::cout << "Wrote output.exr." << std::endl;
#else

  EXRVersion exr_version;

  int ret = ParseEXRVersionFromFile(&exr_version, input_filename);
  if (ret != 0) {
    fprintf(stderr, "Invalid EXR file: %s\n", input_filename);
    return -1;
  }

  printf(
      "version: tiled = %d, long_name = %d, non_image = %d, multipart = %d\n",
      exr_version.tiled, exr_version.long_name, exr_version.non_image,
      exr_version.multipart);

  if (exr_version.multipart) {
    EXRHeader** exr_headers;  // list of EXRHeader pointers.
    int num_exr_headers;

    ret = ParseEXRMultipartHeaderFromFile(&exr_headers, &num_exr_headers,
                                          &exr_version, argv[1], &err);
    if (ret != 0) {
      fprintf(stderr, "Parse EXR err: %s\n", err);
      return ret;
    }

    printf("num parts = %d\n", num_exr_headers);

    for (size_t i = 0; i < static_cast<size_t>(num_exr_headers); i++) {
      const EXRHeader& exr_header = *(exr_headers[i]);

      printf("Part: %lu\n", static_cast<unsigned long>(i));

      printf("dataWindow = %d, %d, %d, %d\n", exr_header.data_window[0],
             exr_header.data_window[1], exr_header.data_window[2],
             exr_header.data_window[3]);
      printf("displayWindow = %d, %d, %d, %d\n", exr_header.display_window[0],
             exr_header.display_window[1], exr_header.display_window[2],
             exr_header.display_window[3]);
      printf("screenWindowCenter = %f, %f\n",
             static_cast<double>(exr_header.screen_window_center[0]),
             static_cast<double>(exr_header.screen_window_center[1]));
      printf("screenWindowWidth = %f\n",
             static_cast<double>(exr_header.screen_window_width));
      printf("pixelAspectRatio = %f\n",
             static_cast<double>(exr_header.pixel_aspect_ratio));
      printf("lineOrder = %d\n", exr_header.line_order);

      if (exr_header.num_custom_attributes > 0) {
        printf("# of custom attributes = %d\n",
               exr_header.num_custom_attributes);
        for (int a = 0; a < exr_header.num_custom_attributes; a++) {
          printf("  [%d] name = %s, type = %s, size = %d\n", a,
                 exr_header.custom_attributes[a].name,
                 exr_header.custom_attributes[a].type,
                 exr_header.custom_attributes[a].size);
          // if (strcmp(exr_header.custom_attributes[i].type, "float") == 0) {
          //  printf("    value = %f\n", *reinterpret_cast<float
          //  *>(exr_header.custom_attributes[i].value));
          //}
        }
      }
    }

    std::vector<EXRImage> images(static_cast<size_t>(num_exr_headers));
    for (size_t i = 0; i < static_cast<size_t>(num_exr_headers); i++) {
      InitEXRImage(&images[i]);
    }

    ret = LoadEXRMultipartImageFromFile(
        &images.at(0), const_cast<const EXRHeader**>(exr_headers),
        static_cast<unsigned int>(num_exr_headers), input_filename, &err);
    if (ret != 0) {
      fprintf(stderr, "Load EXR err: %s\n", err);
      FreeEXRErrorMessage(err);
      return ret;
    }

    printf("Loaded %d part images\n", num_exr_headers);
    printf(
        "There is no saving feature for multi-part images, thus just exit an "
        "application...\n");

    for (size_t i = 0; i < static_cast<size_t>(num_exr_headers); i++) {
      FreeEXRImage(&images.at(i));
    }

    for (size_t i = 0; i < static_cast<size_t>(num_exr_headers); i++) {
      FreeEXRHeader(exr_headers[i]);
      free(exr_headers[i]);
    }
    free(exr_headers);

  } else {  // single-part EXR

    EXRHeader exr_header;
    InitEXRHeader(&exr_header);

    ret =
        ParseEXRHeaderFromFile(&exr_header, &exr_version, input_filename, &err);
    if (ret != 0) {
      fprintf(stderr, "Parse single-part EXR err: %s\n", err);
      FreeEXRErrorMessage(err);
      return ret;
    }

    printf("dataWindow = %d, %d, %d, %d\n", exr_header.data_window[0],
           exr_header.data_window[1], exr_header.data_window[2],
           exr_header.data_window[3]);
    printf("displayWindow = %d, %d, %d, %d\n", exr_header.display_window[0],
           exr_header.display_window[1], exr_header.display_window[2],
           exr_header.display_window[3]);
    printf("screenWindowCenter = %f, %f\n",
           static_cast<double>(exr_header.screen_window_center[0]),
           static_cast<double>(exr_header.screen_window_center[1]));
    printf("screenWindowWidth = %f\n",
           static_cast<double>(exr_header.screen_window_width));
    printf("pixelAspectRatio = %f\n",
           static_cast<double>(exr_header.pixel_aspect_ratio));
    printf("lineOrder = %d\n", exr_header.line_order);

    if (exr_header.num_custom_attributes > 0) {
      printf("# of custom attributes = %d\n", exr_header.num_custom_attributes);
      for (int i = 0; i < exr_header.num_custom_attributes; i++) {
        printf("  [%d] name = %s, type = %s, size = %d\n", i,
               exr_header.custom_attributes[i].name,
               exr_header.custom_attributes[i].type,
               exr_header.custom_attributes[i].size);
        // if (strcmp(exr_header.custom_attributes[i].type, "float") == 0) {
        //  printf("    value = %f\n", *reinterpret_cast<float
        //  *>(exr_header.custom_attributes[i].value));
        //}
      }
    }

    // Read HALF channel as FLOAT.
    for (int i = 0; i < exr_header.num_channels; i++) {
      if (exr_header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
        exr_header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
      }
    }

    EXRImage exr_image;
    InitEXRImage(&exr_image);

    ret = LoadEXRImageFromFile(&exr_image, &exr_header, input_filename, &err);
    if (ret != 0) {
      fprintf(stderr, "Load EXR err: %s\n", err);
      FreeEXRHeader(&exr_header);
      FreeEXRErrorMessage(err);
      return ret;
    }

    printf("EXR: %d x %d\n", exr_image.width, exr_image.height);

    for (int i = 0; i < exr_header.num_channels; i++) {
      printf("pixelType[%d]: %s\n", i, GetPixelType(exr_header.pixel_types[i]));
      printf("chan[%d] = %s\n", i, exr_header.channels[i].name);
      printf("requestedPixelType[%d]: %s\n", i,
             GetPixelType(exr_header.requested_pixel_types[i]));
    }

#if 0  // example to write custom attribute
    int version_minor = 3;
    exr_header.num_custom_attributes = 1;
    exr_header.custom_attributes = reinterpret_cast<EXRAttribute *>(malloc(sizeof(EXRAttribute) * exr_header.custom_attributes));
    strcpy(exr_header.custom_attributes[0].name, "tinyexr_version_minor");
    exr_header.custom_attributes[0].name[strlen("tinyexr_version_minor")] = '\0';
    strcpy(exr_header.custom_attributes[0].type, "int");
    exr_header.custom_attributes[0].type[strlen("int")] = '\0';
    exr_header.custom_attributes[0].size = sizeof(int);
    exr_header.custom_attributes[0].value = (unsigned char*)malloc(sizeof(int));
    memcpy(exr_header.custom_attributes[0].value, &version_minor, sizeof(int));
#endif

    if (exr_header.tiled) {
      TiledImageToScanlineImage(&exr_image, &exr_header);
    }

    exr_header.compression_type = TINYEXR_COMPRESSIONTYPE_NONE;

#ifdef TEST_ZFP_COMPRESSION
    // Assume input image is FLOAT pixel type.
    for (int i = 0; i < exr_header.num_channels; i++) {
      exr_header.channels[i].pixel_type = TINYEXR_PIXELTYPE_FLOAT;
      exr_header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
    }

    unsigned char zfp_compression_type = TINYEXR_ZFP_COMPRESSIONTYPE_RATE;
    double zfp_compression_rate = 4;
    exr_header.num_custom_attributes = 2;
    strcpy(exr_header.custom_attributes[0].name, "zfpCompressionType");
    exr_header.custom_attributes[0].name[strlen("zfpCompressionType")] = '\0';
    exr_header.custom_attributes[0].size = 1;
    exr_header.custom_attributes[0].value =
        (unsigned char*)malloc(sizeof(unsigned char));
    exr_header.custom_attributes[0].value[0] = zfp_compression_type;

    strcpy(exr_header.custom_attributes[1].name, "zfpCompressionRate");
    exr_header.custom_attributes[1].name[strlen("zfpCompressionRate")] = '\0';
    exr_header.custom_attributes[1].size = sizeof(double);
    exr_header.custom_attributes[1].value =
        (unsigned char*)malloc(sizeof(double));
    memcpy(exr_header.custom_attributes[1].value, &zfp_compression_rate,
           sizeof(double));
    exr_header.compression_type = TINYEXR_COMPRESSIONTYPE_ZFP;
#endif

    ret = SaveEXRImageToFile(&exr_image, &exr_header, outfilename, &err);
    if (ret != 0) {
      fprintf(stderr, "Save EXR err: %s\n", err);
      FreeEXRHeader(&exr_header);
      FreeEXRErrorMessage(err);
      return ret;
    }
    printf("Saved exr file. [ %s ] \n", outfilename);

    FreeEXRHeader(&exr_header);
    FreeEXRImage(&exr_image);
  }
#endif

  return ret;
}
