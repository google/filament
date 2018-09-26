#----------------------------------------------------------------------------
#
#	This confidential and proprietary software may be used only as
#	authorised by a licensing agreement from ARM Limited
#	(C) COPYRIGHT 2011-2012 ARM Limited
#	ALL RIGHTS RESERVED
#
#	The entire notice above must be reproduced on all authorised
#	copies and copies may only be made to the extent permitted
#	by a licensing agreement from ARM Limited.
#
#	@brief	Make file to build ASTC codec.
#
#----------------------------------------------------------------------------

SOURCES = \
    astc_averages_and_directions.cpp \
    astc_block_sizes2.cpp \
    astc_color_quantize.cpp \
    astc_color_unquantize.cpp \
    astc_compress_symbolic.cpp \
    astc_decompress_symbolic.cpp \
    astc_encoding_choice_error.cpp \
    astc_find_best_partitioning.cpp \
    astc_ideal_endpoints_and_weights.cpp \
    astc_image_load_store.cpp \
    astc_integer_sequence.cpp \
    astc_ktx_dds.cpp \
    astc_kmeans_partitioning.cpp \
    astc_partition_tables.cpp \
    astc_percentile_tables.cpp \
    astc_pick_best_endpoint_format.cpp \
    astc_quantization.cpp \
    astc_stb_tga.cpp \
    astc_symbolic_physical.cpp \
    astc_weight_align.cpp \
    astc_weight_quant_xfer_tables.cpp \
    astc_compute_variance.cpp \
    astc_toplevel.cpp \
    mathlib.cpp \
    softfloat.cpp \


HEADERS = \
    astc_codec_internals.h \
    mathlib.h \
    softfloat.h \
    stb_image.c


OBJECTS = $(SOURCES:.cpp=.o)


CPPFLAGS = -O3 -Wall -W -Wextra -msse2 -mfpmath=sse


astcenc: $(OBJECTS) stb_image.o
	g++ -o $@ $^ $(CPPFLAGS) -lpthread


$(OBJECTS): %.o: %.cpp $(HEADERS)
	g++ -c -o $@ $< $(CPPFLAGS)


stb_image.o: stb_image.c
	g++ -c -o $@ $< $(CPPFLAGS)


clean:
	rm -f *.o
	rm -f *.obj
	rm -f astcenc
	rm -f astcenc.exe
