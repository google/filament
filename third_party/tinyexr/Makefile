# TinyEXR Makefile
#
#   make            - build the legacy v1 test executable (test_tinyexr)
#   make lib        - build the pure-C11 v3 library (build/libtinyexr3.a)
#   make test-c     - build + run the pure-C11 v3 reader unit test (ASan+UBSan)
#   make c11-gate   - compile every src/*.c as strict C11 -Werror (no C++)
#   make fuzz-corpus - replay regression corpus under ASan+UBSan+LSan
#   make fuzz-corpus-asan - same replay with LSan disabled for ptrace sandboxes
#   make clean

CC  ?= gcc
CXX ?= g++
EMCC ?= emcc
EMAR ?= emar

CFLAGS   ?= -O2
CXXFLAGS ?= -O2 -std=c++11

INCLUDES = -I./deps/miniz
MINIZ_SRC = ./deps/miniz/miniz.c

# ---- legacy v1 single-header test (unchanged) -----------------------------
TARGET = test_tinyexr

.PHONY: all test clean help lib test-c test-c-threads test-c-tsan c11-gate fuzz fuzz-jph fuzz-libdeflate fuzz-corpus fuzz-corpus-asan parse-test wasm freestanding-gate freestanding-zstd-gate examples-c bench bench-compare arm-smoke host-smoke gpu-test vk-test jph-gpu-test bench-gpu-jph texcomp texcomp-arm texcomp-c11-gate texcomp-test texcomp-bench texcomp-astc-psnr texcomp-astc-arm-smoke texcomp-astc-arm-gate texcomp-astc-hdr-gate texcomp-xbc7-gate texcomp-uni-gate texcomp-bc6h-gate texcomp-wasm texcomp-wasm-simd wasm-texcomp wasm-texcomp-simd

all: $(TARGET)

$(TARGET): test_tinyexr.cc miniz.o
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< miniz.o $(LDFLAGS)

miniz.o: $(MINIZ_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

test: $(TARGET)
	./$(TARGET) asakusa.exr

# ---- aggregate: all self-contained tool gates -----------------------------
# CI entry point for tools/ (texcomp, resize/tir, texpipe, envmap). `tools-test`
# runs the pure-C11 gates only (no C++ / no astcenc); `tools-test-all` also runs
# the astcenc conformance cross-checks (astcenc is vendored in deps/, but the
# gate compiles its C++ so it needs a C++ toolchain).
.PHONY: tools-test tools-test-all
tools-test: texcomp-c11-gate texcomp-test texcomp-uni-gate texcomp-xbc7-gate \
            texcomp-bc6h-gate \
            resize-c11-gate resize-test \
            texpipe-c11-gate texpipe-test \
            envmap-c11-gate envmap-test envmap-pbr-test
	@echo "tools-test: all self-contained tool gates passed"

tools-test-all: tools-test texcomp-astc-hdr-gate texcomp-astc-arm-gate
	@echo "tools-test-all: all tool gates (incl. astcenc cross-checks) passed"

# ---- pure-C11 v3 library + tests ------------------------------------------
V3_INC   = -Iinclude -Isrc -Ideps/zstd
V3_CSTD  = -std=c11
V3_WARN  = -Wall -Wextra -Werror
V3_DEFS  =
V3_SRC   = $(wildcard src/*.c)
V3_OBJ   = $(patsubst src/%.c,build/%.o,$(V3_SRC))
# Freestanding core: everything except the optional stdio layer, the spectral
# helpers (hosted-only convenience), and the (freestanding-only) mem/str impls.
V3_CORE_SRC = $(filter-out src/exr_stdio.c src/exr_freestanding.c src/exr_spectral.c src/exr_gpu_cuda.c src/exr_vk_vulkan.c,$(V3_SRC))
ZSTD_SRC = deps/zstd/tinyexr_zstd.c
ZSTD_OBJ = build/tinyexr_zstd.o
V3_TEST_OBJ = $(patsubst src/%.c,build/test-%.o,$(V3_SRC))
SAN      = -fsanitize=address,undefined

# ---- zlib (DEFLATE) backend for ZIP/ZIPS/PXR24 ----------------------------
# DEFLATE = auto | libdeflate | intree    (default: auto)
#   auto / libdeflate : compile the vendored libdeflate (deps/libdeflate, MIT -
#                       see deps/libdeflate/COPYING) AND make it the runtime
#                       default - it is faster on natural-image data. Both
#                       codecs are linked; switch at runtime with the public
#                       exr_zlib_set_backend(). EXR_ZLIB_DEFAULT_LIBDEFLATE=1
#                       seeds the static default (see src/exr_codec.c).
#   intree            : in-tree pure-C codec only, no external dependency.
# The freestanding and wasm targets never define EXR_USE_LIBDEFLATE (their flag
# sets / V3_CORE_SRC omit it), so they always use the in-tree codec regardless.
# Legacy: LIBDEFLATE=1 is kept as an alias for DEFLATE=libdeflate.
# NOTE: run `make clean` when changing DEFLATE (object flags are not tracked).
DEFLATE ?= auto
ifeq ($(LIBDEFLATE),1)
  DEFLATE = libdeflate
endif
LD_OBJ =
LD_TEST_OBJ =
ifneq ($(DEFLATE),intree)
  V3_DEFS += -DEXR_USE_LIBDEFLATE -DEXR_ZLIB_DEFAULT_LIBDEFLATE=1
  V3_INC  += -Ideps/libdeflate
  # Just the zlib (DEFLATE) path: no crc32/gzip. The x86/arm cpu_features files
  # self-guard by arch, so compiling both is safe everywhere.
  LD_SRC = deps/libdeflate/lib/adler32.c \
           deps/libdeflate/lib/deflate_compress.c \
           deps/libdeflate/lib/deflate_decompress.c \
           deps/libdeflate/lib/zlib_compress.c \
           deps/libdeflate/lib/zlib_decompress.c \
           deps/libdeflate/lib/utils.c \
           deps/libdeflate/lib/x86/cpu_features.c \
           deps/libdeflate/lib/arm/cpu_features.c
  LD_OBJ      = $(patsubst deps/libdeflate/%.c,build/libdeflate/%.o,$(LD_SRC))
  LD_TEST_OBJ = $(patsubst deps/libdeflate/%.c,build/test-libdeflate/%.o,$(LD_SRC))
endif

# ---- optional C11-threads multithreading (default OFF; serial is the default)
# Build any target with THREADS=1 to enable per-block parallel encode/decode
# (src/exr_thread.c, uses <threads.h>). Default and freestanding builds stay
# single-threaded. NOTE: run `make clean` when toggling THREADS.
THREADS ?= 0
THREAD_LIBS =
ifeq ($(THREADS),1)
  V3_DEFS    += -DEXR_USE_THREADS
  THREAD_LIBS = -pthread          # C11 threads need pthreads on glibc < 2.34
endif

# ---- optional CUDA GPU backend (default OFF; runtime dlopen via cuew) -------
# Build any target with CUDA=1 to compile the GPU backend (src/exr_gpu_cuda.c +
# third_party/cuew). The CUDA driver and NVRTC are resolved at runtime via cuew
# (dlopen) so NO CUDA SDK is needed at build time and we link only -ldl (never
# -lcuda/-lnvrtc). Without CUDA=1 the backend compiles as inert stubs.
# NOTE: run `make clean` when toggling CUDA (object flags are not tracked).
CUDA ?= 0
CUEW_OBJ =
CUDA_LIBS =
ifeq ($(CUDA),1)
  V3_DEFS += -DEXR_USE_CUDA
  V3_INC  += -Ithird_party/cuew
  CUEW_OBJ  = build/cuew.o
  CUDA_LIBS = -ldl
endif

# ---- optional Vulkan GPU backend (default OFF; runtime dlopen via vkew) -----
# Build any target with VULKAN=1 to compile the Vulkan compute backend
# (src/exr_vk_vulkan.c + third_party/vkew). libvulkan is resolved at runtime via
# vkew (dlopen) so NO Vulkan SDK is needed at build time and we link only -ldl
# (never -lvulkan). Compute shaders are precompiled SPIR-V embedded in
# src/exr_vk_shaders.spv.inc. Without VULKAN=1 the backend compiles as inert
# stubs. NOTE: run `make clean` when toggling VULKAN (object flags not tracked).
VULKAN ?= 0
VKEW_OBJ =
VULKAN_LIBS =
ifeq ($(VULKAN),1)
  V3_DEFS += -DEXR_USE_VULKAN
  V3_INC  += -Ithird_party/vkew
  VKEW_OBJ    = build/vkew.o
  VULKAN_LIBS = -ldl
endif

build:
	@mkdir -p build

# cuew (Apache-2.0, third-party: warnings off).
build/cuew.o: third_party/cuew/cuew.c third_party/cuew/cuew.h | build
	$(CC) -Ithird_party/cuew -O2 -g -w -c $< -o $@

# vkew (Vulkan loader, third-party style: warnings off).
build/vkew.o: third_party/vkew/vkew.c third_party/vkew/vkew.h | build
	$(CC) -Ithird_party/vkew -O2 -g -w -c $< -o $@

# GPU backend TU: extra prereqs (public header, kernels, cuew) so edits rebuild.
build/exr_gpu_cuda.o: src/exr_gpu_cuda.c include/exr_gpu.h include/exr.h \
                      src/exr_internal.h src/exr_gpu_kernels.cuh.inc \
                      src/exr_gpu_jph_kernels.cuh.inc | build
	$(CC) $(V3_CSTD) $(V3_WARN) $(V3_DEFS) $(V3_INC) -O2 -g -c $< -o $@

# Vulkan backend TU: extra prereqs (public header, embedded SPIR-V, vkew).
build/exr_vk_vulkan.o: src/exr_vk_vulkan.c include/exr_vk.h include/exr.h \
                       src/exr_internal.h src/exr_vk_shaders.spv.inc | build
	$(CC) $(V3_CSTD) $(V3_WARN) $(V3_DEFS) $(V3_INC) -O2 -g -c $< -o $@

build/%.o: src/%.c include/exr.h src/exr_internal.h deps/zstd/tinyexr_zstd.h | build
	$(CC) $(V3_CSTD) $(V3_WARN) $(V3_DEFS) $(V3_INC) -O2 -g -c $< -o $@

build/tinyexr_zstd.o: $(ZSTD_SRC) deps/zstd/tinyexr_zstd.h | build
	$(CC) $(V3_CSTD) $(V3_INC) -O2 -g -w -c $< -o $@

# Vendored libdeflate (third-party: warnings off). Release + sanitized variants.
build/libdeflate/%.o: deps/libdeflate/%.c | build
	@mkdir -p $(dir $@)
	$(CC) -Ideps/libdeflate -O3 -g -w -c $< -o $@

build/test-libdeflate/%.o: deps/libdeflate/%.c | build
	@mkdir -p $(dir $@)
	$(CC) -Ideps/libdeflate -O1 -g $(SAN) -w -c $< -o $@

lib: $(V3_OBJ) $(ZSTD_OBJ) $(LD_OBJ) $(CUEW_OBJ) $(VKEW_OBJ)
	$(AR) rcs build/libtinyexr3.a $(V3_OBJ) $(ZSTD_OBJ) $(LD_OBJ) $(CUEW_OBJ) $(VKEW_OBJ)

# GPU backend test (requires CUDA=1; skips at runtime with exit 77 if no device).
EXR_IMAGES ?= $(HOME)/work/openexr-images
gpu-test:
	$(MAKE) clean            # CUDA flag is not object-tracked; rebuild from clean
	$(MAKE) CUDA=1 lib
	$(CC) $(V3_CSTD) -Wall -Wextra -DEXR_USE_CUDA -Iinclude -Ithird_party/cuew \
	  -O2 -g test/gpu/test_exr_gpu.c build/libtinyexr3.a -ldl -lm \
	  -o build/test_exr_gpu
	./build/test_exr_gpu "$(EXR_IMAGES)"; rc=$$?; \
	  if [ $$rc -eq 77 ]; then echo "gpu-test: SKIPPED (no CUDA device)"; exit 0; \
	  else exit $$rc; fi

# HTJ2K GPU block-coder bit-exactness test (CUDA=1; exit 77 = skip, no device).
jph-gpu-test:
	$(MAKE) clean
	$(MAKE) CUDA=1 lib
	$(CC) $(V3_CSTD) -Wall -Wextra -DEXR_USE_CUDA -Iinclude -Isrc -Ideps/zstd \
	  -Ithird_party/cuew -O2 -g test/gpu/test_exr_jph_gpu.c build/libtinyexr3.a \
	  -ldl -lm -o build/test_exr_jph_gpu
	./build/test_exr_jph_gpu "$(EXR_IMAGES)"; rc=$$?; \
	  if [ $$rc -eq 77 ]; then echo "jph-gpu-test: SKIPPED (no CUDA device)"; exit 0; \
	  else exit $$rc; fi

# Vulkan backend test (requires VULKAN=1; skips at runtime with exit 77 if no device).
vk-test:
	$(MAKE) clean            # VULKAN flag is not object-tracked; rebuild from clean
	$(MAKE) VULKAN=1 lib
	$(CC) $(V3_CSTD) -Wall -Wextra -DEXR_USE_VULKAN -Iinclude -Ithird_party/vkew \
	  -O2 -g test/vk/test_exr_vk.c build/libtinyexr3.a -ldl -lm \
	  -o build/test_exr_vk
	./build/test_exr_vk "$(EXR_IMAGES)"; rc=$$?; \
	  if [ $$rc -eq 77 ]; then echo "vk-test: SKIPPED (no Vulkan device)"; exit 0; \
	  else exit $$rc; fi

# Strict pure-C11 gate: the rewrite must never require a C++ compiler.
c11-gate: | build
	@for f in $(V3_SRC); do \
	  echo "  C11  $$f"; \
	  $(CC) $(V3_CSTD) $(V3_WARN) $(V3_INC) -O1 -fsyntax-only $$f || exit 1; \
	done
	@echo "pure-C11 gate: OK"

build/test-%.o: src/%.c include/exr.h src/exr_internal.h deps/zstd/tinyexr_zstd.h | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(V3_DEFS) $(V3_INC) -O1 -g $(SAN) -c $< -o $@

build/test-tinyexr_zstd.o: $(ZSTD_SRC) deps/zstd/tinyexr_zstd.h | build
	$(CC) $(V3_CSTD) $(V3_INC) -O1 -g $(SAN) -w -c $< -o $@

test-c: $(V3_TEST_OBJ) build/test-tinyexr_zstd.o $(LD_TEST_OBJ) test/unit/test_exr_v3.c | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(V3_DEFS) $(V3_INC) -O1 -g $(SAN) \
	  test/unit/test_exr_v3.c $(V3_TEST_OBJ) build/test-tinyexr_zstd.o $(LD_TEST_OBJ) $(THREAD_LIBS) -lm -o build/test_exr_v3
	ASAN_OPTIONS=detect_leaks=0 ./build/test_exr_v3

# ---- tools/texcomp: pure-C11 BC/ETC/ASTC texture compression --------------
# One translation unit per codec, plus texcomp.c for the shared core
# (backend dispatch, options, sizes, DDS/KTX/ASTC containers).
TEXCOMP_INC = -Itools/texcomp/include -Iinclude -Isrc -Iexamples/common
TEXCOMP_SRC = tools/texcomp/src/texcomp.c \
  tools/texcomp/src/texcomp_bc1.c tools/texcomp/src/texcomp_bc3.c \
  tools/texcomp/src/texcomp_bc5.c tools/texcomp/src/texcomp_bc6h.c \
  tools/texcomp/src/texcomp_bc7.c tools/texcomp/src/texcomp_etc2.c \
  tools/texcomp/src/texcomp_eac.c tools/texcomp/src/texcomp_astc.c \
  tools/texcomp/src/texcomp_astc_hdr.c tools/texcomp/src/texcomp_uni.c \
  tools/texcomp/src/texcomp_astc_decode.c
TEXCOMP_HDRS = tools/texcomp/include/texcomp.h tools/texcomp/src/texcomp_internal.h
TEXCOMP_OBJ = $(patsubst tools/texcomp/src/%.c,build/texcomp/%.o,$(TEXCOMP_SRC))
TEXCOMP_TEST_OBJ = $(patsubst tools/texcomp/src/%.c,build/texcomp/test-%.o,$(TEXCOMP_SRC))
TEXCOMP_OPT ?= -O3
TEXCOMP_WASM_OPT ?= -O3
TEXCOMP_WASM_CACHE ?= /tmp/tinyexr-emcc-cache
TEXCOMP_WASM_EMCC = EM_CACHE=$(TEXCOMP_WASM_CACHE) $(EMCC)
TEXCOMP_WASM_COMMON = $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) $(TEXCOMP_WASM_OPT) -DTC_NO_THREADS=1
TEXCOMP_WASM_CLI_WARN = -Wno-unused-function -Wno-macro-redefined
TEXCOMP_WASM_SIMD = -msimd128
TEXCOMP_WASM_EXR_SRC = $(V3_CORE_SRC) src/exr_stdio.c $(ZSTD_SRC)
TEXCOMP_WASM_EXPORTS = ['_tc_result_string','_tc_backend_name','_tc_backend_available_mask','_tc_backend_force_mask','_tc_bc7_options_init','_tc_bc1_options_init','_tc_bc3_options_init','_tc_bc5_options_init','_tc_bc6h_options_init','_tc_etc2_options_init','_tc_astc_options_init','_tc_astc_hdr_options_init','_tc_bc7_compressed_size','_tc_bc1_compressed_size','_tc_bc3_compressed_size','_tc_bc5_compressed_size','_tc_bc6h_compressed_size','_tc_etc2_rgb_compressed_size','_tc_etc2_rgba_compressed_size','_tc_eac_r11_compressed_size','_tc_eac_rg11_compressed_size','_tc_astc_compressed_size','_tc_astc_hdr_compressed_size','_tc_astc_ise_sequence_bitcount','_tc_astc_ise_encode_bits','_tc_bc7_compress_rgba8','_tc_bc7_decompress_rgba8','_tc_bc1_compress_rgba8','_tc_bc3_compress_rgba8','_tc_bc5_compress_rg8','_tc_bc5_compress_rgba8','_tc_bc6h_compress_rgb32f','_tc_etc2_compress_rgba8','_tc_eac_compress_rgba8','_tc_astc_compress_rgba8','_tc_astc_hdr_compress_rgbf','_tc_dds_bc7_size','_tc_dds_bc1_size','_tc_dds_bc3_size','_tc_dds_bc5_size','_tc_dds_bc6h_size','_tc_ktx_etc2_size','_tc_astc_file_size','_tc_dds_write_bc7_memory','_tc_dds_write_bc1_memory','_tc_dds_write_bc3_memory','_tc_dds_write_bc5_memory','_tc_dds_write_bc6h_memory','_tc_ktx_write_etc2_memory','_tc_ktx_write_eac_memory','_tc_astc_write_file_memory','_malloc','_free']
TEXCOMP_WASM_RUNTIME = ['HEAPU8','HEAPF32','HEAP32','HEAPU32','UTF8ToString','stringToUTF8','lengthBytesUTF8','ccall','cwrap']

build/texcomp:
	@mkdir -p build/texcomp

build/texcomp/wasm build/texcomp/wasm-simd $(TEXCOMP_WASM_CACHE):
	@mkdir -p $@

build/texcomp/%.o: tools/texcomp/src/%.c $(TEXCOMP_HDRS) | build/texcomp
	$(CC) $(V3_CSTD) $(V3_WARN) $(TEXCOMP_INC) $(TEXCOMP_OPT) -g -c $< -o $@

build/texcomp/test-%.o: tools/texcomp/src/%.c $(TEXCOMP_HDRS) | build/texcomp
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) -O1 -g $(SAN) -c $< -o $@

texcomp: lib $(TEXCOMP_OBJ) tools/texcomp/src/texcomp_cli.c | build/texcomp
	$(AR) rcs build/libtexcomp.a $(TEXCOMP_OBJ)
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) $(V3_DEFS) $(V3_INC) -O2 -g \
	  tools/texcomp/src/texcomp_cli.c build/libtexcomp.a build/libtinyexr3.a \
	  -pthread -lm -o build/texcomp/texcomp

TEXCOMP_WASM_OBJ = $(patsubst tools/texcomp/src/%.c,build/texcomp/wasm/%.o,$(TEXCOMP_SRC))
TEXCOMP_WASM_SIMD_OBJ = $(patsubst tools/texcomp/src/%.c,build/texcomp/wasm-simd/%.o,$(TEXCOMP_SRC))

build/texcomp/wasm/%.o: tools/texcomp/src/%.c $(TEXCOMP_HDRS) | build/texcomp/wasm $(TEXCOMP_WASM_CACHE)
	$(TEXCOMP_WASM_EMCC) $(TEXCOMP_WASM_COMMON) -c $< -o $@

build/texcomp/wasm-simd/%.o: tools/texcomp/src/%.c $(TEXCOMP_HDRS) | build/texcomp/wasm-simd $(TEXCOMP_WASM_CACHE)
	$(TEXCOMP_WASM_EMCC) $(TEXCOMP_WASM_COMMON) $(TEXCOMP_WASM_SIMD) -c $< -o $@

build/texcomp/wasm/libtexcomp.a: $(TEXCOMP_WASM_OBJ)
	$(EMAR) rcs $@ $^

build/texcomp/wasm-simd/libtexcomp.a: $(TEXCOMP_WASM_SIMD_OBJ)
	$(EMAR) rcs $@ $^

build/texcomp/wasm/texcomp.mjs: $(TEXCOMP_WASM_OBJ) | $(TEXCOMP_WASM_CACHE)
	$(TEXCOMP_WASM_EMCC) $(TEXCOMP_WASM_OPT) $(TEXCOMP_WASM_OBJ) \
	  -s FILESYSTEM=0 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 \
	  -s EXPORT_ES6=1 -s ENVIRONMENT=web,node \
	  -s "EXPORTED_FUNCTIONS=$(TEXCOMP_WASM_EXPORTS)" \
	  -s "EXPORTED_RUNTIME_METHODS=$(TEXCOMP_WASM_RUNTIME)" \
	  -o $@

build/texcomp/wasm-simd/texcomp.mjs: $(TEXCOMP_WASM_SIMD_OBJ) | $(TEXCOMP_WASM_CACHE)
	$(TEXCOMP_WASM_EMCC) $(TEXCOMP_WASM_OPT) $(TEXCOMP_WASM_SIMD) $(TEXCOMP_WASM_SIMD_OBJ) \
	  -s FILESYSTEM=0 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 \
	  -s EXPORT_ES6=1 -s ENVIRONMENT=web,node \
	  -s "EXPORTED_FUNCTIONS=$(TEXCOMP_WASM_EXPORTS)" \
	  -s "EXPORTED_RUNTIME_METHODS=$(TEXCOMP_WASM_RUNTIME)" \
	  -o $@

build/texcomp/wasm/texcomp_cli.js: tools/texcomp/src/texcomp_cli.c $(TEXCOMP_SRC) $(TEXCOMP_HDRS) $(TEXCOMP_WASM_EXR_SRC) | build/texcomp/wasm $(TEXCOMP_WASM_CACHE)
	$(TEXCOMP_WASM_EMCC) $(TEXCOMP_WASM_COMMON) $(TEXCOMP_WASM_CLI_WARN) $(V3_INC) \
	  tools/texcomp/src/texcomp_cli.c $(TEXCOMP_SRC) $(TEXCOMP_WASM_EXR_SRC) \
	  -s FORCE_FILESYSTEM=1 -s NODERAWFS=1 -s ALLOW_MEMORY_GROWTH=1 \
	  -s EXIT_RUNTIME=1 -s ENVIRONMENT=node \
	  -o $@

build/texcomp/wasm-simd/texcomp_cli.js: tools/texcomp/src/texcomp_cli.c $(TEXCOMP_SRC) $(TEXCOMP_HDRS) $(TEXCOMP_WASM_EXR_SRC) | build/texcomp/wasm-simd $(TEXCOMP_WASM_CACHE)
	$(TEXCOMP_WASM_EMCC) $(TEXCOMP_WASM_COMMON) $(TEXCOMP_WASM_CLI_WARN) $(TEXCOMP_WASM_SIMD) $(V3_INC) \
	  tools/texcomp/src/texcomp_cli.c $(TEXCOMP_SRC) $(TEXCOMP_WASM_EXR_SRC) \
	  -s FORCE_FILESYSTEM=1 -s NODERAWFS=1 -s ALLOW_MEMORY_GROWTH=1 \
	  -s EXIT_RUNTIME=1 -s ENVIRONMENT=node \
	  -o $@

texcomp-wasm: build/texcomp/wasm/libtexcomp.a build/texcomp/wasm/texcomp.mjs build/texcomp/wasm/texcomp_cli.js
	@echo "built build/texcomp/wasm/libtexcomp.a, texcomp.mjs/.wasm, texcomp_cli.js/.wasm"

texcomp-wasm-simd: build/texcomp/wasm-simd/libtexcomp.a build/texcomp/wasm-simd/texcomp.mjs build/texcomp/wasm-simd/texcomp_cli.js
	@echo "built build/texcomp/wasm-simd/libtexcomp.a, texcomp.mjs/.wasm, texcomp_cli.js/.wasm"

wasm-texcomp: texcomp-wasm

wasm-texcomp-simd: texcomp-wasm-simd

texcomp-c11-gate: | build/texcomp
	for f in $(TEXCOMP_SRC); do \
	  $(CC) $(V3_CSTD) $(V3_WARN) $(TEXCOMP_INC) -O1 -fsyntax-only $$f || exit 1; \
	done
	$(CC) $(V3_CSTD) $(V3_WARN) $(TEXCOMP_INC) $(V3_INC) -O1 -fsyntax-only tools/texcomp/src/texcomp_cli.c
	@echo "texcomp pure-C11 gate: OK"

texcomp-test: $(TEXCOMP_TEST_OBJ) tools/texcomp/test/test_texcomp.c | build/texcomp
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) -O1 -g $(SAN) -pthread \
	  tools/texcomp/test/test_texcomp.c $(TEXCOMP_TEST_OBJ) -lm -o build/test_texcomp
	./build/test_texcomp

texcomp-uni-gate: $(TEXCOMP_OBJ) tools/texcomp/test/uni_gate.c | build/texcomp
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) -O2 -g \
	  tools/texcomp/test/uni_gate.c $(TEXCOMP_OBJ) -lm -o build/texcomp/uni_gate
	./build/texcomp/uni_gate

# BC6H conformance + quality gate: encode HDR, decode every block with an
# independent reference decoder (bcdec port) and check PSNR vs source.
texcomp-bc6h-gate: $(TEXCOMP_OBJ) tools/texcomp/test/bc6h_gate.c | build/texcomp
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) -Itools/texcomp/test -O2 -g \
	  tools/texcomp/test/bc6h_gate.c $(TEXCOMP_OBJ) -lm -o build/texcomp/bc6h_gate
	./build/texcomp/bc6h_gate

# BC6H/BC7 pipeline quality gate: loads real EXR images, encodes/decodes,
# checks PSNR + SSIM. Argument: path to openexr-images directory.
texcomp-pipeline-gate: $(TEXCOMP_OBJ) tools/texcomp/test/bc6h_pipeline_gate.c \
                        tools/texcomp/test/tc_ssim.h \
                        tools/texcomp/test/tc_ssim_gauss11.inc | build/texcomp
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) -Itools/texcomp/test -O2 -g \
	  tools/texcomp/test/bc6h_pipeline_gate.c $(TEXCOMP_OBJ) build/libtinyexr3.a \
	  -lm -o build/texcomp/bc6h_pipeline_gate
	./build/texcomp/bc6h_pipeline_gate $(OPENEXR_IMAGES_DIR)

texcomp-bench: $(TEXCOMP_OBJ) tools/texcomp/bench/texcomp_bench.c | build/texcomp
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) -O3 \
	  tools/texcomp/bench/texcomp_bench.c $(TEXCOMP_OBJ) -lm -o build/texcomp_bench
	./build/texcomp_bench

texcomp-astc-psnr: $(TEXCOMP_OBJ) tools/texcomp/bench/texcomp_psnr.c tools/texcomp/test/astc_ref_decode.h | build/texcomp
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) -Itools/texcomp/test -O2 \
	  tools/texcomp/bench/texcomp_psnr.c $(TEXCOMP_OBJ) -lm -o build/texcomp_psnr
	./build/texcomp_psnr

# ---- vendored Arm astcenc backend (deps/astcenc, Apache-2.0) ---------------
# Full port of the upstream Arm encoder, selectable at runtime with
# `texcomp-arm --encoder arm`. Portable SSE2 baseline on x86, NEON on
# aarch64; the default `make texcomp` stays pure C11 with no C++ parts.
ASTCENC_LIB_SRC = $(wildcard deps/astcenc/*.cpp)
ASTCENC_LIB_OBJ = $(patsubst deps/astcenc/%.cpp,build/astcenc/%.o,$(ASTCENC_LIB_SRC))
ASTCENC_HOST_ARCH ?= $(shell uname -m)
ifneq ($(filter aarch64 arm64,$(ASTCENC_HOST_ARCH)),)
  ASTCENC_DEFS = -DASTCENC_SSE=0 -DASTCENC_AVX=0 -DASTCENC_NEON=1 -DASTCENC_SVE=0 -DASTCENC_POPCNT=0 -DASTCENC_F16C=0
else
  ASTCENC_DEFS = -DASTCENC_SSE=20 -DASTCENC_AVX=0 -DASTCENC_NEON=0 -DASTCENC_SVE=0 -DASTCENC_POPCNT=0 -DASTCENC_F16C=0
endif

build/astcenc/%.o: deps/astcenc/%.cpp
	@mkdir -p build/astcenc
	$(CXX) -std=c++14 -O3 -g -w $(ASTCENC_DEFS) -Ideps/astcenc -c $< -o $@

texcomp-arm: lib $(TEXCOMP_OBJ) $(ASTCENC_LIB_OBJ) tools/texcomp/src/texcomp_cli.c | build/texcomp
	$(AR) rcs build/libtexcomp_astcenc.a $(ASTCENC_LIB_OBJ)
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) $(V3_DEFS) $(V3_INC) -O2 -g \
	  -DTEXCOMP_HAVE_ASTCENC -Ideps/astcenc \
	  -c tools/texcomp/src/texcomp_cli.c -o build/texcomp/texcomp_cli_arm.o
	$(CXX) build/texcomp/texcomp_cli_arm.o $(TEXCOMP_OBJ) \
	  build/libtexcomp_astcenc.a build/libtinyexr3.a \
	  -pthread -lm -o build/texcomp/texcomp-arm

ASTCENC ?= /tmp/astc-encoder/build/Source/astcenc-native

texcomp-astc-arm-smoke: texcomp
	@test -x "$(ASTCENC)" || { echo "set ASTCENC=/path/to/astcenc-native"; exit 77; }
	./build/texcomp/texcomp -i issue40.exr -o build/texcomp/arm_smoke_6x6.astc --format astc --astc-block 6x6 --quality normal
	"$(ASTCENC)" -dl build/texcomp/arm_smoke_6x6.astc build/texcomp/arm_smoke_6x6.png -silent

# Self-contained CI gate for the vendored astcenc backend: builds the C++
# encoder, encodes one image with both the pure-C `tc` path and astcenc, and
# cross-checks their PSNR using astcenc's own decoder. No external binaries or
# image assets, so it runs anywhere the backend compiles.
texcomp-astc-arm-gate: $(TEXCOMP_OBJ) $(ASTCENC_LIB_OBJ) tools/texcomp/test/astc_arm_xcheck.c | build/texcomp
	$(AR) rcs build/libtexcomp_astcenc.a $(ASTCENC_LIB_OBJ)
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) -Itools/texcomp/test \
	  -DTEXCOMP_HAVE_ASTCENC -Ideps/astcenc -O2 -g -c \
	  tools/texcomp/test/astc_arm_xcheck.c -o build/texcomp/astc_arm_xcheck.o
	$(CXX) build/texcomp/astc_arm_xcheck.o $(TEXCOMP_OBJ) \
	  build/libtexcomp_astcenc.a -lm -o build/texcomp/astc_arm_xcheck
	./build/texcomp/astc_arm_xcheck

# Self-contained CI gate for the ASTC HDR encoder: encodes deterministic HDR
# images with the pure-C tc encoder and verifies them with astcenc's conformant
# HDR decoder (const-colour round-trip + gradient PSNR floor).
# Basis Universal transcoder validation gate. Vendored from
# https://github.com/BinomialLLC/basis_universal (transcoder/basisu_transcoder.cpp)
# into deps/basisu/. C++ build like astcenc; skip if files not present.
BASISU_DIR ?= deps/basisu
BASISU_HDR = $(BASISU_DIR)/basisu_transcoder.h
BASISU_SRC = $(BASISU_DIR)/basisu_transcoder.cpp

BASISU_DEFS = -DBASISD_SUPPORT_KTX2=1 -DBASISD_SUPPORT_KTX2_ZSTD=0

texcomp-basis-gate: tools/texcomp/test/basis_validate.c | build/texcomp
	@test -f "$(BASISU_SRC)" || { echo "basis-validate: vendored transcoder not found (cp from https://github.com/BinomialLLC/basis_universal)"; exit 77; }
	$(CXX) -std=c++17 -Wall -Wextra -fno-strict-aliasing $(BASISU_DEFS) -I$(BASISU_DIR) -O2 -g -c \
	  tools/texcomp/test/basis_validate.c -o build/texcomp/basis_validate.o
	$(CXX) -std=c++17 $(BASISU_DEFS) -I$(BASISU_DIR) -O2 -g -c \
	  $(BASISU_SRC) -o build/texcomp/basisu_transcoder.o
	$(CXX) build/texcomp/basis_validate.o build/texcomp/basisu_transcoder.o -lm -o build/texcomp/basis_validate
	./build/texcomp/basis_validate
	@echo "basis-validate: OK"

texcomp-astc-hdr-gate: $(TEXCOMP_OBJ) $(ASTCENC_LIB_OBJ) tools/texcomp/test/astc_hdr_xcheck.c | build/texcomp
	$(AR) rcs build/libtexcomp_astcenc.a $(ASTCENC_LIB_OBJ)
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) -Itools/texcomp/test \
	  -DTEXCOMP_HAVE_ASTCENC -Ideps/astcenc -O2 -g -c \
	  tools/texcomp/test/astc_hdr_xcheck.c -o build/texcomp/astc_hdr_xcheck.o
	$(CXX) build/texcomp/astc_hdr_xcheck.o $(TEXCOMP_OBJ) \
	  build/libtexcomp_astcenc.a -lm -o build/texcomp/astc_hdr_xcheck
	./build/texcomp/astc_hdr_xcheck

# xbc7: BC7 windowed RDO + zstd container. C gate checks the RDO improves
# zstd compressibility (and rdo=0 is a no-op); the CLI step checks the
# encode->transcode round-trip is bit-exact standard BC7.
texcomp-xbc7-gate: lib $(TEXCOMP_OBJ) texcomp tools/texcomp/test/xbc7_gate.c tools/texcomp/test/bc7_ref_decode.h | build/texcomp
	$(AR) rcs build/libtexcomp.a $(TEXCOMP_OBJ)
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXCOMP_INC) -Itools/texcomp/test -Ideps/zstd -O2 -g \
	  tools/texcomp/test/xbc7_gate.c build/libtexcomp.a build/libtinyexr3.a \
	  -pthread -lm -o build/texcomp/xbc7_gate
	./build/texcomp/xbc7_gate
	@echo "--- xbc7 CLI encode -> transcode round-trip ---"
	./build/texcomp/texcomp -i asakusa.png -o build/texcomp/rt.xbc7 \
	  --format xbc7 --rdo 16 --raw build/texcomp/rt_enc.bc7
	./build/texcomp/texcomp -i build/texcomp/rt.xbc7 -o build/texcomp/rt.dds \
	  --raw build/texcomp/rt_dec.bc7
	@git diff --no-index --quiet build/texcomp/rt_enc.bc7 build/texcomp/rt_dec.bc7 \
	  && echo "xbc7 CLI round-trip: OK (transcode is bit-exact BC7)" \
	  || { echo "FAIL: xbc7 round-trip differs"; exit 1; }

# ---- tools/texpipe: resize-aware texture compression ----------------------
# Ties tir (resize) + texcomp (block compression) into content-aware mip
# chains serialized to multi-mip DDS / KTX2 containers. Pure C11 library
# (no <stdio.h> in src/); only texpipe_cli.c does file I/O.
TEXPIPE_INC = -Itools/texpipe/include -Itools/resize/include \
  -Itools/texcomp/include -Iinclude -Isrc -Iexamples/common
TEXPIPE_LIB_SRC = tools/texpipe/src/texpipe.c tools/texpipe/src/texpipe_mip.c \
  tools/texpipe/src/texpipe_alpha.c tools/texpipe/src/texpipe_cube.c \
  tools/texpipe/src/texpipe_octa.c tools/texpipe/src/texpipe_disp.c \
  tools/texpipe/src/texpipe_normal.c tools/texpipe/src/texpipe_container.c
TEXPIPE_HDRS = tools/texpipe/include/texpipe.h tools/texpipe/src/texpipe_internal.h
TEXPIPE_OBJ = $(patsubst tools/texpipe/src/%.c,build/texpipe/%.o,$(TEXPIPE_LIB_SRC))

.PHONY: texpipe texpipe-c11-gate texpipe-test

build/texpipe:
	@mkdir -p build/texpipe

build/texpipe/%.o: tools/texpipe/src/%.c $(TEXPIPE_HDRS) | build/texpipe
	$(CC) $(V3_CSTD) $(V3_WARN) $(TEXPIPE_INC) -O2 -g -c $< -o $@

# Full CLI: base image -> content-aware mip chain -> compressed container.
texpipe: lib resize-lib texcomp $(TEXPIPE_OBJ) tools/texpipe/src/texpipe_cli.c | build/texpipe
	$(AR) rcs build/libtexpipe.a $(TEXPIPE_OBJ)
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXPIPE_INC) $(V3_DEFS) $(V3_INC) -O2 -g \
	  tools/texpipe/src/texpipe_cli.c build/libtexpipe.a build/libtir.a \
	  build/libtexcomp.a build/libtinyexr3.a -pthread -lm \
	  -o build/texpipe/texpipe
	@echo "built build/texpipe/texpipe"

# Strict pure-C11 gate: syntax-check each lib TU with -Werror and forbid
# <stdio.h> outside the CLI.
texpipe-c11-gate: | build/texpipe
	@for f in $(TEXPIPE_LIB_SRC); do \
	  echo "  c11-gate $$f"; \
	  $(CC) $(V3_CSTD) $(V3_WARN) $(TEXPIPE_INC) -O1 -fsyntax-only $$f || exit 1; \
	done
	@bad=`grep -rl --exclude=texpipe_cli.c '<stdio.h>' tools/texpipe/src/ || true`; \
	  if [ -n "$$bad" ]; then echo "FAIL: <stdio.h> in texpipe src: $$bad"; exit 1; fi
	$(CC) $(V3_CSTD) $(V3_WARN) $(TEXPIPE_INC) $(V3_INC) -O1 -fsyntax-only \
	  tools/texpipe/src/texpipe_cli.c
	@echo "texpipe pure-C11 gate: OK"

# Unit tests: per-mip round-trip PSNR (BC7 shipped decoder) + alpha coverage.
texpipe-test: resize-lib texcomp tools/texpipe/test/test_texpipe.c $(TEXPIPE_HDRS) | build/texpipe
	$(CC) $(V3_CSTD) -Wall -Wextra $(TEXPIPE_INC) -Itools/texcomp/test -O1 -g $(SAN) -pthread \
	  tools/texpipe/test/test_texpipe.c $(TEXPIPE_LIB_SRC) build/libtir.a \
	  build/libtexcomp.a -lm -o build/test_texpipe
	./build/test_texpipe

# ---- tools/envmap: environment-map projections, SH, spherical gaussians ----
# Pure C11. Links tir + texcomp + texpipe + libtinyexr3 (CLI does HDR EXR I/O).
ENVMAP_INC = -Itools/envmap/include -Itools/resize/include \
  -Itools/texcomp/include -Itools/texpipe/include -Iinclude -Isrc -Iexamples/common
ENVMAP_LIB_SRC = tools/envmap/src/envmap_proj.c tools/envmap/src/envmap_sample.c \
  tools/envmap/src/envmap_sh.c tools/envmap/src/envmap_sg.c \
  tools/envmap/src/envmap_ibl.c
ENVMAP_HDRS = tools/envmap/include/envmap.h
ENVMAP_OBJ = $(patsubst tools/envmap/src/%.c,build/envmap/%.o,$(ENVMAP_LIB_SRC))

.PHONY: envmap envmap-c11-gate envmap-test envmap-pbr-test

build/envmap:
	@mkdir -p build/envmap

build/envmap/%.o: tools/envmap/src/%.c $(ENVMAP_HDRS) | build/envmap
	$(CC) $(V3_CSTD) $(V3_WARN) $(ENVMAP_INC) -O2 -g -c $< -o $@

envmap: lib resize-lib texcomp texpipe $(ENVMAP_OBJ) tools/envmap/src/envmap_cli.c | build/envmap
	$(AR) rcs build/libenvmap.a $(ENVMAP_OBJ)
	$(CC) $(V3_CSTD) -Wall -Wextra $(ENVMAP_INC) $(V3_DEFS) $(V3_INC) -O2 -g \
	  tools/envmap/src/envmap_cli.c build/libenvmap.a build/libtexpipe.a \
	  build/libtir.a build/libtexcomp.a build/libtinyexr3.a -pthread -lm \
	  -o build/envmap/envmap
	@echo "built build/envmap/envmap"

envmap-c11-gate: | build/envmap
	@for f in $(ENVMAP_LIB_SRC); do \
	  echo "  c11-gate $$f"; \
	  $(CC) $(V3_CSTD) $(V3_WARN) $(ENVMAP_INC) -O1 -fsyntax-only $$f || exit 1; \
	done
	@bad=`grep -rl --exclude=envmap_cli.c '<stdio.h>' tools/envmap/src/ || true`; \
	  if [ -n "$$bad" ]; then echo "FAIL: <stdio.h> in envmap src: $$bad"; exit 1; fi
	$(CC) $(V3_CSTD) $(V3_WARN) $(ENVMAP_INC) $(V3_INC) -O1 -fsyntax-only \
	  tools/envmap/src/envmap_cli.c
	@echo "envmap pure-C11 gate: OK"

envmap-test: resize-lib tools/envmap/test/test_envmap.c $(ENVMAP_HDRS) | build/envmap
	$(CC) $(V3_CSTD) -Wall -Wextra $(ENVMAP_INC) -O1 -g $(SAN) -pthread \
	  tools/envmap/test/test_envmap.c $(ENVMAP_LIB_SRC) build/libtir.a -lm \
	  -o build/test_envmap
	./build/test_envmap

# PBR validation harness: shade under IBL with source vs BC7-decoded material.
envmap-pbr-test: resize-lib texcomp tools/envmap/test/test_pbr.c $(ENVMAP_HDRS) | build/envmap
	$(CC) $(V3_CSTD) -Wall -Wextra $(ENVMAP_INC) -O1 -g $(SAN) -pthread \
	  tools/envmap/test/test_pbr.c $(ENVMAP_LIB_SRC) build/libtir.a \
	  build/libtexcomp.a -lm -o build/test_pbr
	./build/test_pbr

# Build + run the unit tests with multithreading enabled (parity + race checks).
test-c-threads:
	$(MAKE) test-c THREADS=1

# Thread sanitizer over the threaded build (no ASan; proves no data races).
# NOTE: requires a ThreadSanitizer that instruments C11 <threads.h>; some
# glibc/TSan combos only intercept pthread_* and crash on thrd_create/mtx_*.
# The threaded build also runs cleanly under ASan+UBSan via `make test-c-threads`.
test-c-tsan: | build
	$(CC) $(V3_CSTD) -Wall -Wextra -DEXR_USE_THREADS $(V3_INC) -O1 -g \
	  -fsanitize=thread test/unit/test_exr_v3.c $(V3_SRC) $(ZSTD_SRC) \
	  -pthread -lm -o build/test_exr_v3_tsan
	./build/test_exr_v3_tsan

bench: $(V3_OBJ) $(ZSTD_OBJ) $(LD_OBJ) benchmark/bench.c | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(V3_DEFS) $(V3_INC) -O3 \
	  benchmark/bench.c $(V3_OBJ) $(ZSTD_OBJ) $(LD_OBJ) $(THREAD_LIBS) -lm -o build/bench
	./build/bench

# ---- HTJ2K GPU vs CPU throughput (CUDA backend) ---------------------------
# Builds a CUDA lib and times whole-image HTJ2K decode/encode CPU vs GPU.
# Exit 77 = skipped (no CUDA device). Override the image with EXR_BENCH_IMG=...
EXR_BENCH_IMG ?= asakusa.exr
bench-gpu-jph:
	$(MAKE) clean
	$(MAKE) CUDA=1 lib
	$(CC) $(V3_CSTD) -Wall -Wextra -DEXR_USE_CUDA -Iinclude -Isrc -Ideps/zstd \
	  -Ithird_party/cuew -O3 benchmark/bench_gpu_jph.c build/libtinyexr3.a \
	  -ldl -lm -o build/bench_gpu_jph
	./build/bench_gpu_jph "$(EXR_BENCH_IMG)"; rc=$$?; \
	  if [ $$rc -eq 77 ]; then echo "bench-gpu-jph: SKIPPED (no CUDA device)"; exit 0; \
	  else exit $$rc; fi

# ---- tinyexr-vs-OpenEXR comparison (needs a built OpenEXR) -----------------
# Override OPENEXR_ROOT / OPENEXR_BUILD if your tree lives elsewhere. Extra
# files: make bench-compare ARGS="img1.exr img2.exr"
OPENEXR_ROOT  ?= $(HOME)/work/openexr
OPENEXR_BUILD ?= $(OPENEXR_ROOT)/_build
OPENEXR_INC    = -I$(OPENEXR_ROOT)/src/lib/OpenEXR -I$(OPENEXR_ROOT)/src/lib/OpenEXRCore \
                 -I$(OPENEXR_ROOT)/src/lib/Iex -I$(OPENEXR_ROOT)/src/lib/IlmThread \
                 -I$(OPENEXR_BUILD)/cmake $(shell pkg-config --cflags Imath 2>/dev/null)
OPENEXR_LIBDIR = $(OPENEXR_BUILD)/src/lib
OPENEXR_LIBS   = -L$(OPENEXR_LIBDIR)/OpenEXR -L$(OPENEXR_LIBDIR)/OpenEXRCore \
                 -L$(OPENEXR_LIBDIR)/Iex -L$(OPENEXR_LIBDIR)/IlmThread \
                 -lOpenEXR-4_0 -lOpenEXRCore-4_0 -lIex-4_0 -lIlmThread-4_0 -pthread
OPENEXR_LDPATH = $(OPENEXR_LIBDIR)/OpenEXR:$(OPENEXR_LIBDIR)/OpenEXRCore:$(OPENEXR_LIBDIR)/Iex:$(OPENEXR_LIBDIR)/IlmThread

# The tinyexr side (bench_tx.c) is compiled as C because exr.h and OpenEXR's
# C core declare the same global enum names and cannot share a translation unit.
build/bench_tx.o: benchmark/bench_tx.c benchmark/bench_tx.h include/exr.h | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(V3_INC) -O3 -c benchmark/bench_tx.c -o $@

bench-compare: $(V3_OBJ) $(ZSTD_OBJ) $(LD_OBJ) build/bench_tx.o benchmark/bench_compare.cpp | build
	@test -d $(OPENEXR_LIBDIR)/OpenEXR || { \
	  echo "OpenEXR build not found at $(OPENEXR_BUILD)"; \
	  echo "build OpenEXR first, or set OPENEXR_ROOT=/path/to/openexr"; exit 1; }
	$(CXX) -std=c++14 -Wall -Ibenchmark $(OPENEXR_INC) -O3 \
	  benchmark/bench_compare.cpp build/bench_tx.o $(V3_OBJ) $(ZSTD_OBJ) $(LD_OBJ) \
	  $(OPENEXR_LIBS) $(THREAD_LIBS) -lm -o build/bench_compare
	LD_LIBRARY_PATH=$(OPENEXR_LDPATH) ./build/bench_compare $(ARGS)

# Coverage-guided fuzzer (clang+libFuzzer over the whole library).
#   ./build/fuzz_v3 -max_total_time=60 test/unit/regression
fuzz: test/fuzzer/fuzz_v3.c | build
	clang $(V3_CSTD) $(V3_INC) -O1 -g -w -fsanitize=fuzzer,address,undefined \
	  test/fuzzer/fuzz_v3.c $(V3_SRC) $(ZSTD_SRC) -lm -o build/fuzz_v3
	@echo "built build/fuzz_v3 - e.g. ./build/fuzz_v3 -max_total_time=60 test/unit/regression"

# Same fuzzer but with libdeflate as the default zlib backend, so the shipped
# DEFLATE=auto decode path (ZIP/ZIPS/PXR24 -> libdeflate) is fuzzed too.
fuzz-libdeflate: test/fuzzer/fuzz_v3.c | build
	clang $(V3_CSTD) -DEXR_USE_LIBDEFLATE -DEXR_ZLIB_DEFAULT_LIBDEFLATE=1 \
	  $(V3_INC) -Ideps/libdeflate -O1 -g -w -fsanitize=fuzzer,address,undefined \
	  test/fuzzer/fuzz_v3.c $(V3_SRC) $(ZSTD_SRC) \
	  deps/libdeflate/lib/adler32.c deps/libdeflate/lib/deflate_compress.c \
	  deps/libdeflate/lib/deflate_decompress.c deps/libdeflate/lib/zlib_compress.c \
	  deps/libdeflate/lib/zlib_decompress.c deps/libdeflate/lib/utils.c \
	  deps/libdeflate/lib/x86/cpu_features.c deps/libdeflate/lib/arm/cpu_features.c \
	  -lm -o build/fuzz_v3_libdeflate
	@echo "built build/fuzz_v3_libdeflate (libdeflate default backend)"

# HTJ2K (JPH) encode+decode+round-trip fuzzer.
#   ./build/fuzz_jph -max_total_time=600 test/fuzzer/corpus_jph
fuzz-jph: test/fuzzer/fuzz_jph.c | build
	clang $(V3_CSTD) $(V3_INC) -O1 -g -w -fsanitize=fuzzer,address,undefined \
	  test/fuzzer/fuzz_jph.c $(V3_SRC) $(ZSTD_SRC) -lm -o build/fuzz_jph
	@echo "built build/fuzz_jph"

# Deterministic corpus replay for fuzz_jph (no libFuzzer needed).
fuzz-jph-corpus: test/fuzzer/fuzz_jph.c | build
	clang $(V3_CSTD) -Wall $(V3_INC) -O1 -g $(SAN) -DEXR_JPH_FUZZ_STANDALONE \
	  test/fuzzer/fuzz_jph.c $(V3_SRC) $(ZSTD_SRC) -lm -o build/fuzz_jph_replay

# Deterministic corpus replay under ASan+UBSan (no libFuzzer needed; CI gate).
fuzz-corpus: $(V3_TEST_OBJ) build/test-tinyexr_zstd.o $(LD_TEST_OBJ) test/fuzzer/fuzz_v3.c | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(V3_INC) -O1 -g $(SAN) -DEXR_FUZZ_STANDALONE \
	  test/fuzzer/fuzz_v3.c $(V3_TEST_OBJ) build/test-tinyexr_zstd.o $(LD_TEST_OBJ) -lm \
	  -o build/fuzz_replay
	./build/fuzz_replay test/unit/regression/* asakusa.exr deepscanline.exr

# Some local sandboxes/debug wrappers use ptrace. LeakSanitizer cannot run
# under ptrace, so this target preserves ASan+UBSan corpus coverage there while
# keeping fuzz-corpus as the strict LSan gate for CI/native hosts.
fuzz-corpus-asan: $(V3_TEST_OBJ) build/test-tinyexr_zstd.o $(LD_TEST_OBJ) test/fuzzer/fuzz_v3.c | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(V3_INC) -O1 -g $(SAN) -DEXR_FUZZ_STANDALONE \
	  test/fuzzer/fuzz_v3.c $(V3_TEST_OBJ) build/test-tinyexr_zstd.o $(LD_TEST_OBJ) -lm \
	  -o build/fuzz_replay
	ASAN_OPTIONS=detect_leaks=0 ./build/fuzz_replay test/unit/regression/* asakusa.exr deepscanline.exr

# Parse/load every *.exr under $(EXR_IMAGES) and classify PASS/XFAIL/FAIL.
# Override the corpus dir with: make parse-test EXR_IMAGES=/path/to/images
EXR_IMAGES ?= $(HOME)/work/openexr-images
PARSE_HARNESS = test/v3/parse_harness

parse-test: lib
	$(CC) $(V3_CSTD) -Wall -Wextra -Iinclude -O2 \
	  test/v3/parse_harness.c build/libtinyexr3.a -lm -o $(PARSE_HARNESS)
	python3 test/v3/parse-tester.py --harness $(PARSE_HARNESS) $(EXR_IMAGES)

# ---- native example (uses the optional stdio layer) -----------------------
examples-c: lib
	$(CC) $(V3_CSTD) -Wall -Wextra -Iinclude -O2 \
	  examples/exrinfo/exrinfo.c build/libtinyexr3.a -o build/exrinfo
	@echo "built build/exrinfo - e.g. ./build/exrinfo asakusa.exr"

# ---- Emscripten WASM build of the v3 C API --------------------------------
# Pure-C exports, no filesystem. Produces an ES6 module + wasm. Needs emcc on
# PATH (override with EMCC=...). The freestanding core links cleanly with no FS.
WASM_EXPORTS = ['_exrw_decode_rgba','_exrw_encode_rgba','_exrw_free','_malloc','_free']
WASM_RUNTIME = ['HEAPU8','HEAPF32','HEAP32','HEAPU32']
wasm: | build
	$(EMCC) -O3 $(V3_INC) -w \
	  $(V3_CORE_SRC) $(ZSTD_SRC) examples/wasm/exr_wasm.c \
	  -s FILESYSTEM=0 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 \
	  -s EXPORT_ES6=1 -s ENVIRONMENT=web,node \
	  -s "EXPORTED_FUNCTIONS=$(WASM_EXPORTS)" \
	  -s "EXPORTED_RUNTIME_METHODS=$(WASM_RUNTIME)" \
	  -o build/exr_v3.mjs
	@echo "built build/exr_v3.mjs + build/exr_v3.wasm"

# ---- ARM (aarch64 NEON) cross build + emulated run ------------------------
# Cross-compile the SIMD smoke test for aarch64 and run it under an emulator to
# confirm the NEON path builds and is bit-identical to scalar. Static link so
# the emulator needs no guest sysroot. Override the toolchain/emulator as needed:
#   make arm-smoke ARM_CC=aarch64-linux-gnu-gcc ARM_QEMU=qemu-aarch64
ARM_CC   ?= aarch64-linux-gnu-gcc-13
ARM_QEMU ?= qemu-aarch64-static
ARM_SMOKE_SRC = $(filter-out src/exr_zstd.c,$(V3_SRC))

arm-smoke: test/v3/neon_smoke.c | build
	$(ARM_CC) -static -march=armv8-a $(V3_CSTD) -Wall -Wextra -DEXR_NO_ZSTD \
	  $(V3_INC) -O2 test/v3/neon_smoke.c $(ARM_SMOKE_SRC) -lm \
	  -o build/neon_smoke_arm
	@file build/neon_smoke_arm | sed 's/^/  /'
	$(ARM_QEMU) ./build/neon_smoke_arm

# Same smoke, built/run natively (sanity-checks the host SIMD tier).
host-smoke: test/v3/neon_smoke.c | build
	$(CC) $(V3_CSTD) -Wall -Wextra -DEXR_NO_ZSTD $(V3_INC) -O2 \
	  test/v3/neon_smoke.c $(ARM_SMOKE_SRC) -lm -o build/neon_smoke_host
	./build/neon_smoke_host

# ---- freestanding gate ----------------------------------------------------
# Compile the core with only stdint/stddef/limits, prove there are no forbidden
# libc dependencies (nm scan), and run a functional memory round-trip.
# Opt-in zstd DECODE in the freestanding build (default off). Adds the vendored
# zstd amalgamation (~hundreds of KB) and routes decode through zstd's no-malloc
# static-DCtx API with malloc/calloc/free stubbed out, so the forbidden-symbol
# scan still passes. Decode-only: zstd ENCODE stays UNSUPPORTED in freestanding.
EXR_FREESTANDING_ZSTD ?= 0
ifeq ($(EXR_FREESTANDING_ZSTD),1)
  FS_ZSTD_CFG  = -DEXR_ZSTD_DECODE_ONLY
  FS_ZSTD_SKIP =
  FS_ZSTD_OBJ  = build/fs-tinyexr_zstd.o
  FS_SMOKE_CFG = -DEXR_FREESTANDING_ZSTD -Itest/v3
else
  FS_ZSTD_CFG  = -DEXR_NO_ZSTD
  FS_ZSTD_SKIP = src/exr_zstd.c
  FS_ZSTD_OBJ  =
  FS_SMOKE_CFG =
endif
FS_FLAGS = -DEXR_FREESTANDING $(FS_ZSTD_CFG) -ffreestanding -fno-builtin \
           -fno-stack-protector $(V3_CSTD) $(V3_WARN) $(V3_INC) -O2 -g
# Default: exclude the zstd glue (EXR_NO_ZSTD), dispatch returns UNSUPPORTED, no
# amalgamation pulled in. EXR_FREESTANDING_ZSTD=1 includes both (decode path).
FS_CORE_SRC = $(filter-out $(FS_ZSTD_SKIP),$(V3_CORE_SRC))
FS_OBJ = $(patsubst src/%.c,build/fs-%.o,$(FS_CORE_SRC)) build/fs-exr_freestanding.o \
         $(FS_ZSTD_OBJ)
FS_FORBIDDEN = fopen|fread|fwrite|fseek|ftell|fclose|fprintf|printf|snprintf|malloc|calloc|realloc|free|abort|exit|qsort|exp|log|pow

build/fs-%.o: src/%.c include/exr.h src/exr_internal.h | build
	$(CC) $(FS_FLAGS) -c $< -o $@

# Freestanding zstd amalgamation: no-malloc (static-DCtx) build with the libc
# allocator entry points stubbed to NULL/no-op (the static decode path never
# calls them), so the object carries no forbidden symbols.
build/fs-tinyexr_zstd.o: deps/zstd/tinyexr_zstd.c deps/zstd/tinyexr_zstd.h | build
	$(CC) -DEXR_FREESTANDING -ffreestanding -fno-builtin -fno-stack-protector \
	  -DNDEBUG -DZSTD_DEPS_MALLOC -D'ZSTD_malloc(s)=((void*)0)' \
	  -D'ZSTD_calloc(n,s)=((void*)0)' -D'ZSTD_free(p)=((void)0)' \
	  -Ideps/zstd $(V3_CSTD) -O2 -g -w -c $< -o $@

freestanding-gate: $(FS_OBJ) test/v3/freestanding_smoke.c | build
	@echo "  scan: only exr_stdio.c may include <stdio.h> (GPU backends are hosted-only)"
	@bad=`grep -rl '<stdio.h>' src/ | grep -vE 'src/(exr_stdio|exr_gpu_cuda|exr_vk_vulkan)\.c' || true`; \
	  if [ -n "$$bad" ]; then echo "  FAIL: stdio leaked into: $$bad"; exit 1; fi
	@echo "  scan: no forbidden libc symbols referenced by the freestanding core"
	@for o in $(FS_OBJ); do \
	  hit=`nm -u $$o 2>/dev/null | awk '{print $$NF}' | grep -wE '$(FS_FORBIDDEN)' || true`; \
	  if [ -n "$$hit" ]; then echo "  FAIL: $$o references:" $$hit; exit 1; fi; \
	done
	@echo "  run: freestanding-compiled core + custom-allocator round-trip"
	$(CC) $(V3_CSTD) -Wall -Wextra $(FS_SMOKE_CFG) -Iinclude -Isrc -O2 \
	  test/v3/freestanding_smoke.c $(FS_OBJ) -o build/fs_smoke
	./build/fs_smoke
	@echo "freestanding gate: OK"

# Regenerate the freestanding zstd decode fixture (test/v3/fs_zstd_blob.inc).
gen-fs-zstd-blob: lib
	$(CC) $(V3_CSTD) -Iinclude -O2 test/v3/gen_fs_zstd_blob.c build/libtinyexr3.a \
	  -lm -o build/gen_fs_zstd_blob
	./build/gen_fs_zstd_blob > test/v3/fs_zstd_blob.inc
	@echo "regenerated test/v3/fs_zstd_blob.inc"

# Freestanding gate with the opt-in zstd DECODE path enabled (proves the
# stubbed-malloc amalgamation stays forbidden-symbol-clean and decodes).
# The fs-*.o objects are not flag-tracked, so remove any built with different
# zstd flags (e.g. a prior `make freestanding-gate`) to force a correct rebuild.
freestanding-zstd-gate:
	rm -f build/fs-*.o build/fs_smoke
	$(MAKE) freestanding-gate EXR_FREESTANDING_ZSTD=1

# ---- tocio (sandbox: tiny OpenColorIO config engine + codegen) ------------
# Pure-C11, freestanding, no external deps. Lives outside src/ so it has its own
# build/gate targets. Mirrors the v3 freestanding discipline.
TOC_INC      = -Isandbox/tocio/include -Isandbox/tocio/src
TOC_SRC      = $(wildcard sandbox/tocio/src/*.c)
TOC_OBJ      = $(patsubst sandbox/tocio/src/%.c,build/toc-%.o,$(TOC_SRC))
TOC_HDRS     = sandbox/tocio/include/tocio.h sandbox/tocio/src/toc_internal.h
# Freestanding core: everything except the optional hosted stdio loader and the
# hosted JIT (needs OS executable memory).
TOC_CORE_SRC = $(filter-out sandbox/tocio/src/toc_stdio.c sandbox/tocio/src/toc_jit.c,$(TOC_SRC))
TOC_FS_OBJ   = $(patsubst sandbox/tocio/src/%.c,build/toc-fs-%.o,$(TOC_CORE_SRC))
TOC_FS_FORBIDDEN = $(FS_FORBIDDEN)

.PHONY: tocio-lib tocio-c11-gate tocio-freestanding-gate tocio-test

build/toc-%.o: sandbox/tocio/src/%.c $(TOC_HDRS) | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(TOC_INC) -O2 -g $(SAN) -c $< -o $@

tocio-lib: $(TOC_OBJ)
	$(AR) rcs build/libtocio.a $(TOC_OBJ)
	@echo "built build/libtocio.a"

tocio-c11-gate: | build
	@for f in $(TOC_SRC); do \
	  echo "  C11  $$f"; \
	  $(CC) $(V3_CSTD) $(V3_WARN) $(TOC_INC) -ffp-contract=off -O1 -fsyntax-only $$f || exit 1; \
	done
	@echo "tocio pure-C11 gate: OK"

build/toc-fs-%.o: sandbox/tocio/src/%.c $(TOC_HDRS) | build
	$(CC) -DTOC_FREESTANDING -ffreestanding -fno-builtin -fno-stack-protector \
	  $(V3_CSTD) $(V3_WARN) $(TOC_INC) -ffp-contract=off -O2 -g -c $< -o $@

tocio-freestanding-gate: $(TOC_FS_OBJ) sandbox/tocio/tests/toc_fs_smoke.c | build
	@echo "  scan: only toc_stdio.c may include <stdio.h>"
	@bad=`grep -rl '<stdio.h>' sandbox/tocio/src/ | grep -v 'toc_stdio.c' || true`; \
	  if [ -n "$$bad" ]; then echo "  FAIL: stdio leaked into: $$bad"; exit 1; fi
	@echo "  scan: no forbidden libc symbols in the freestanding core"
	@for o in $(TOC_FS_OBJ); do \
	  hit=`nm -u $$o 2>/dev/null | awk '{print $$NF}' | grep -wE '$(TOC_FS_FORBIDDEN)' || true`; \
	  if [ -n "$$hit" ]; then echo "  FAIL: $$o references:" $$hit; exit 1; fi; \
	done
	@echo "  run: freestanding-compiled core + custom-allocator round-trip"
	$(CC) $(V3_CSTD) -Wall -Wextra $(TOC_INC) -O2 \
	  sandbox/tocio/tests/toc_fs_smoke.c $(TOC_FS_OBJ) -o build/toc_fs_smoke
	./build/toc_fs_smoke
	@echo "tocio freestanding gate: OK"

tocio-test: | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(TOC_INC) -O1 -g $(SAN) \
	  sandbox/tocio/tests/toc_test.c $(TOC_SRC) -lm -ldl -o build/toc_test
	ASAN_OPTIONS=detect_leaks=0 ./build/toc_test

# Validate tocio against the REAL AcademySoftwareFoundation ACES OCIO configs:
# parse each config, build every transform, and compare results to golden values
# captured from PyOpenColorIO (the C++ reference engine). The configs live under
# sandbox/tocio/ref (fetch them first); the golden TSV is committed.
#   make tocio-fetch-ref    # download configs + reference repos into ref/
#   make tocio-gen-golden   # regenerate the golden TSV (needs PyOpenColorIO)
TOC_REFDIR    = sandbox/tocio/ref/configs
TOC_GOLDEN    = sandbox/tocio/tests/golden/aces_golden.tsv
.PHONY: tocio-validate tocio-fetch-ref tocio-gen-golden
tocio-validate: | build
	@if [ ! -d "$(TOC_REFDIR)" ]; then \
	  echo "ref configs missing - run 'make tocio-fetch-ref' first"; exit 2; fi
	$(CC) $(V3_CSTD) -Wall -Wextra $(TOC_INC) -O1 -g $(SAN) \
	  sandbox/tocio/tests/toc_validate.c $(TOC_SRC) -lm -ldl -o build/toc_validate
	ASAN_OPTIONS=detect_leaks=0 ./build/toc_validate $(TOC_GOLDEN) $(TOC_REFDIR)

tocio-fetch-ref:
	bash sandbox/tocio/scripts/fetch_ocio_ref.sh

tocio-gen-golden:
	@sp=$$(echo sandbox/tocio/ref/.pyoracle/lib/python*/site-packages); \
	  PYTHONPATH="$$sp" python3 sandbox/tocio/scripts/gen_golden.py

# Interpreter throughput benchmark (scalar vs SIMD per op). -O2, no sanitizers.
.PHONY: tocio-bench
tocio-bench: | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(TOC_INC) -O2 \
	  sandbox/tocio/tests/toc_bench.c $(TOC_SRC) -lm -ldl -o build/toc_bench
	./build/toc_bench

# ---- tocio ARM64 (aarch64 NEON) cross build + qemu test ---------------------
# Cross-compile the full tocio test suite and run under qemu-aarch64.
# toc_jit.c has a native AArch64/NEON backend (emits A64 code to executable
# memory); under qemu-user the JIT may report TOC_ERROR_UNSUPPORTED and skip.
.PHONY: tocio-arm-test
tocio-arm-test: | build
	$(ARM_CC) -static -march=armv8-a $(V3_CSTD) -Wall -Wextra $(TOC_INC) -O2 \
	  sandbox/tocio/tests/toc_test.c $(TOC_SRC) -lm -o build/toc_test_arm
	@file build/toc_test_arm | sed 's/^/  /'
	$(ARM_QEMU) ./build/toc_test_arm

# ---- tocio WASM (Emscripten ES6 module for the web viewer) -----------------
TOCW_EXPORTS = ['_tocw_parse','_tocw_free_config','_tocw_processor','_tocw_processor_view','_tocw_free_ops','_tocw_apply','_tocw_emit_glsl','_tocw_jit_glsl','_tocw_emit_metal','_tocw_emit_c','_tocw_free_str','_tocw_num_colorspaces','_tocw_colorspace_name','_malloc','_free']
TOCW_RUNTIME = ['HEAPU8','HEAPF32','HEAP32','UTF8ToString','stringToUTF8','lengthBytesUTF8']
.PHONY: wasm-tocio wasm-tocio-test
# toc_wasm.c references the JIT (tocw_jit_glsl); include toc_jit.c (inert stub
# under wasm32) so the module links.
wasm-tocio: | build
	$(EMCC) -O3 $(TOC_INC) -w \
	  $(TOC_CORE_SRC) sandbox/tocio/src/toc_jit.c sandbox/tocio/wasm/toc_wasm.c \
	  -s FILESYSTEM=0 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 \
	  -s EXPORT_ES6=1 -s ENVIRONMENT=web,node \
	  -s "EXPORTED_FUNCTIONS=$(TOCW_EXPORTS)" \
	  -s "EXPORTED_RUNTIME_METHODS=$(TOCW_RUNTIME)" \
	  -o build/tocio.mjs
	@echo "built build/tocio.mjs + build/tocio.wasm"

wasm-tocio-test: wasm-tocio
	node sandbox/tocio/wasm/test.mjs

# ---- tocio web demo: ONE module with EXR decode + tocio + JIT(->GLSL) -------
# Combines the v3 EXR decoder (exrw_decode_rgba) and the tocio engine (incl.
# toc_jit.c, which compiles to its inert stub under wasm32 -> "JIT outputs GLSL")
# into build/tocio_demo.mjs for web/tocio/. toc_stdio.c is excluded (no FS).
TOCDEMO_SRC = $(TOC_CORE_SRC) sandbox/tocio/src/toc_jit.c sandbox/tocio/wasm/toc_wasm.c
TOCDEMO_EXPORTS = ['_exrw_decode_rgba','_exrw_free','_tocw_parse','_tocw_free_config','_tocw_processor','_tocw_processor_view','_tocw_free_ops','_tocw_apply','_tocw_emit_glsl','_tocw_jit_glsl','_tocw_emit_c','_tocw_free_str','_tocw_num_colorspaces','_tocw_colorspace_name','_tocw_num_displays','_tocw_display_name','_tocw_num_views','_tocw_view_name','_tocw_role','_malloc','_free']
TOCDEMO_RUNTIME = ['HEAPU8','HEAPF32','HEAP32','UTF8ToString','stringToUTF8','lengthBytesUTF8']
.PHONY: wasm-tocio-demo
wasm-tocio-demo: | build
	$(EMCC) -O3 $(V3_INC) $(TOC_INC) -w \
	  $(V3_CORE_SRC) $(ZSTD_SRC) examples/wasm/exr_wasm.c \
	  $(TOCDEMO_SRC) \
	  -s FILESYSTEM=0 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 \
	  -s EXPORT_ES6=1 -s ENVIRONMENT=web,node \
	  -s "EXPORTED_FUNCTIONS=$(TOCDEMO_EXPORTS)" \
	  -s "EXPORTED_RUNTIME_METHODS=$(TOCDEMO_RUNTIME)" \
	  -o web/tocio/tocio_demo.mjs
	@echo "built web/tocio/tocio_demo.mjs + .wasm"

# ---- tools/resize (tir: standalone SIMD image resize library) --------------
# Pure C11, no dependency on the v3 core; only the CLI/bench link
# libtinyexr3.a (for EXR file I/O). See tools/resize/README.md.
TIR_INC  = -Itools/resize/include -Itools/resize/src
TIR_SRC  = $(wildcard tools/resize/src/*.c)
TIR_OBJ  = $(patsubst tools/resize/src/%.c,build/tir-%.o,$(TIR_SRC))
TIR_HDRS = tools/resize/include/tir.h tools/resize/src/tir_internal.h
# Only tir_kernels_sve.c is built with +sve so the HWCAP runtime gate stays
# sound (everything else remains baseline). Opt in with TIR_SVE=1 on aarch64.
TIR_SVE_FLAGS =
ifeq ($(TIR_SVE),1)
TIR_SVE_FLAGS = -march=armv8-a+sve
endif

.PHONY: resize-lib resize-c11-gate resize-test resize-test-asan \
        resize-test-threads resize-test-tsan resize-test-f16 resize-bench \
        resize-cli resize-cli-asan resize-fuzz resize-fuzz-corpus \
        resize-arm-test resize-sve-test

build/tir-%.o: tools/resize/src/%.c $(TIR_HDRS) | build
	$(CC) $(V3_CSTD) $(V3_WARN) $(TIR_INC) -O2 -g -c $< -o $@
build/tir-tir_kernels_sve.o: tools/resize/src/tir_kernels_sve.c $(TIR_HDRS) | build
	$(CC) $(V3_CSTD) $(V3_WARN) $(TIR_INC) $(TIR_SVE_FLAGS) -O2 -g -c $< -o $@

resize-lib: $(TIR_OBJ)
	$(AR) rcs build/libtir.a $(TIR_OBJ)
	@echo "built build/libtir.a"

resize-c11-gate: | build
	@for f in $(TIR_SRC); do \
	  echo "  C11  $$f"; \
	  $(CC) $(V3_CSTD) $(V3_WARN) $(TIR_INC) -O1 -fsyntax-only $$f || exit 1; \
	done
	@echo "  scan: library sources must not include <stdio.h>"
	@bad=`grep -rl '<stdio.h>' tools/resize/src/ || true`; \
	  if [ -n "$$bad" ]; then echo "  FAIL: stdio leaked into: $$bad"; exit 1; fi
	@echo "resize pure-C11 gate: OK"

resize-test: | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(TIR_INC) -O1 -g $(SAN) \
	  tools/resize/tests/tir_test.c $(TIR_SRC) -lm -o build/tir_test
	ASAN_OPTIONS=detect_leaks=0 ./build/tir_test

# Same unit tests, but with LeakSanitizer on (the library is single-alloc /
# single-free, so the suite must be leak-clean) and threads enabled so the
# banded whole-image paths run under ASan+UBSan+LSan too.
resize-test-asan: | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(TIR_INC) -DTIR_ENABLE_THREADS -pthread \
	  -O1 -g $(SAN) \
	  tools/resize/tests/tir_test.c $(TIR_SRC) -lm -o build/tir_test_asan
	ASAN_OPTIONS=detect_leaks=1 ./build/tir_test_asan

resize-test-threads: | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(TIR_INC) -DTIR_ENABLE_THREADS -pthread \
	  -O1 -g $(SAN) \
	  tools/resize/tests/tir_test.c $(TIR_SRC) -lm -o build/tir_test_mt
	ASAN_OPTIONS=detect_leaks=0 ./build/tir_test_mt

# ThreadSanitizer build. Uses the pthread threading backend
# (-DTIR_THREADS_PTHREAD): TSan does not intercept glibc's C11 thrd_create, so
# a worker would SEGV in __tsan_func_entry before running. The band logic and
# shared-data access pattern are backend-independent, so this validly checks
# the threaded paths for data races.
resize-test-tsan: | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(TIR_INC) \
	  -DTIR_ENABLE_THREADS -DTIR_THREADS_PTHREAD -pthread \
	  -O1 -g -fsanitize=thread \
	  tools/resize/tests/tir_test.c $(TIR_SRC) -lm -o build/tir_test_tsan
	./build/tir_test_tsan

# Exhaustive f16<->f32 converter sweep (all 65536 half codes, every SIMD
# level vs the scalar reference, both directions). Guards the SIMD sNaN
# preservation, which the resize pipeline itself cannot exercise. Runs the
# native levels, then the aarch64 NEON/SVE kernels under qemu when the cross
# toolchain (ARM_CC, default gcc-13; pass ARM_CC=... for another) is present.
resize-test-f16: | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(TIR_INC) -O2 -g \
	  tools/resize/tests/tir_f16_test.c $(TIR_SRC) -lm -o build/tir_f16_test
	./build/tir_f16_test
	@if command -v $(ARM_CC) >/dev/null 2>&1; then \
	  echo "== NEON (qemu) =="; \
	  $(ARM_CC) -static -march=armv8-a $(V3_CSTD) -Wall -Wextra $(TIR_INC) \
	    -O2 tools/resize/tests/tir_f16_test.c $(TIR_SRC) -lm \
	    -o build/tir_f16_test_arm && $(ARM_QEMU) ./build/tir_f16_test_arm; \
	  echo "== SVE (qemu) =="; \
	  $(ARM_CC) -static -march=armv8-a+sve $(V3_CSTD) -Wall -Wextra \
	    $(TIR_INC) -O2 -c tools/resize/src/tir_kernels_sve.c \
	    -o build/tir-sve-kernels-f16.o && \
	  $(ARM_CC) -static -march=armv8-a $(V3_CSTD) -Wall -Wextra $(TIR_INC) \
	    -O2 tools/resize/tests/tir_f16_test.c \
	    $(filter-out tools/resize/src/tir_kernels_sve.c,$(TIR_SRC)) \
	    build/tir-sve-kernels-f16.o -lm -o build/tir_f16_test_sve && \
	  $(ARM_QEMU) -cpu max,sve=on,sve256=on ./build/tir_f16_test_sve; \
	else echo "  ($(ARM_CC) not found; skipping NEON/SVE f16 sweep)"; fi

# STB=1 adds a stb_image_resize2 comparison column; the header is NOT
# vendored - drop stb_image_resize2.h into tools/resize/tests/ first.
TIR_BENCH_DEFS =
ifeq ($(STB),1)
TIR_BENCH_DEFS = -DTIR_BENCH_STB
endif

resize-bench: lib | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(TIR_INC) $(V3_INC) $(TIR_BENCH_DEFS) -O2 -g \
	  tools/resize/tests/tir_bench.c $(TIR_SRC) build/libtinyexr3.a \
	  -lm -o build/tir_bench
	./build/tir_bench

resize-cli: lib resize-lib | build
	$(CC) $(V3_CSTD) $(V3_WARN) $(TIR_INC) -Iinclude -O2 \
	  tools/resize/cli/tir_resize_main.c build/libtir.a build/libtinyexr3.a \
	  -lm -o build/tir_resize
	@echo "built build/tir_resize"

# CLI under ASan+UBSan+LeakSanitizer: a real EXR round-trip plus an error
# path, guarding the CLI's malloc checks and the single-cleanup free path.
resize-cli-asan: lib | build
	$(CC) $(V3_CSTD) $(V3_WARN) $(TIR_INC) -Iinclude -O1 -g $(SAN) \
	  tools/resize/cli/tir_resize_main.c $(TIR_SRC) build/libtinyexr3.a \
	  -lm -o build/tir_resize_asan
	@echo "  round-trip under ASan+LSan (must be leak-clean)"
	ASAN_OPTIONS=detect_leaks=1 ./build/tir_resize_asan asakusa.exr \
	  -o build/tir_asan_out.exr --scale 0.5 --filter lanczos3 \
	  --antiring 1 --clamp-min 0 --stats
	@echo "  error path (save failure) must not leak"
	@ASAN_OPTIONS=detect_leaks=1:exitcode=99 ./build/tir_resize_asan \
	  asakusa.exr -o /nonexistent_dir_zzz/out.exr --scale 0.5; \
	  rc=$$?; if [ $$rc = 99 ]; then echo "  FAIL: leak on error path"; \
	  exit 1; fi; echo "  error path clean (exit $$rc)"
	@echo "resize CLI ASan: OK"

# Coverage-guided libFuzzer harness for the resize library (threads enabled so
# the banded whole-image path is fuzzed too). Needs clang.
resize-fuzz: tools/resize/tests/tir_fuzz.c | build
	clang $(V3_CSTD) $(TIR_INC) -DTIR_ENABLE_THREADS -pthread -O1 -g -w \
	  -fsanitize=fuzzer,address,undefined \
	  tools/resize/tests/tir_fuzz.c $(TIR_SRC) -lm -o build/resize_fuzz
	@echo "built build/resize_fuzz"
	@echo "  run: mkdir -p build/corpus_resize && cp tools/resize/tests/corpus/* \\"
	@echo "       build/corpus_resize/ && ./build/resize_fuzz -max_total_time=300 \\"
	@echo "       build/corpus_resize tools/resize/tests/corpus"

# Deterministic replay under ASan+UBSan (no libFuzzer; CI gate). First replays
# the committed seed corpus, then runs 8000 generated pseudo-random inputs.
resize-fuzz-corpus: tools/resize/tests/tir_fuzz.c | build
	$(CC) $(V3_CSTD) -Wall -Wextra $(TIR_INC) -DTIR_ENABLE_THREADS -pthread \
	  -O1 -g $(SAN) -DTIR_FUZZ_STANDALONE \
	  tools/resize/tests/tir_fuzz.c $(TIR_SRC) -lm -o build/resize_fuzz_replay
	ASAN_OPTIONS=detect_leaks=0 ./build/resize_fuzz_replay \
	  tools/resize/tests/corpus/*
	ASAN_OPTIONS=detect_leaks=0 ./build/resize_fuzz_replay

# Cross-build for AArch64 (NEON kernels) and run under qemu.
resize-arm-test: | build
	$(ARM_CC) -static -march=armv8-a $(V3_CSTD) -Wall -Wextra $(TIR_INC) -O2 \
	  tools/resize/tests/tir_test.c $(TIR_SRC) -lm -o build/tir_test_arm
	$(ARM_QEMU) ./build/tir_test_arm

# Same, with the SVE unit enabled and qemu exposing SVE (256-bit vectors).
resize-sve-test: | build
	$(ARM_CC) -static -march=armv8-a+sve $(V3_CSTD) -Wall -Wextra $(TIR_INC) -O2 \
	  -c tools/resize/src/tir_kernels_sve.c -o build/tir-sve-kernels-arm.o
	$(ARM_CC) -static -march=armv8-a $(V3_CSTD) -Wall -Wextra $(TIR_INC) -O2 \
	  tools/resize/tests/tir_test.c \
	  $(filter-out tools/resize/src/tir_kernels_sve.c,$(TIR_SRC)) \
	  build/tir-sve-kernels-arm.o -lm -o build/tir_test_sve
	$(ARM_QEMU) -cpu max,sve=on,sve256=on ./build/tir_test_sve

clean:
	rm -rf $(TARGET) miniz.o build $(PARSE_HARNESS)

help:
	@echo "make        - legacy v1 test (test_tinyexr)"
	@echo "make lib    - pure-C11 v3 library (build/libtinyexr3.a)"
	@echo "make test-c - run pure-C11 v3 reader unit test (ASan+UBSan)"
	@echo "make test-c-threads - unit tests with multithreading (THREADS=1)"
	@echo "make test-c-tsan - threaded unit tests under ThreadSanitizer"
	@echo "make c11-gate - strict C11 -Werror compile of all src/*.c"
	@echo "make bench  - codec/SIMD throughput benchmark (incl. HTJ2K SIMD tiers)"
	@echo "make tools-test - run all self-contained tool gates (texcomp/resize/texpipe/envmap)"
	@echo "make tools-test-all - tools-test + astcenc conformance cross-checks (needs C++)"
	@echo "make texcomp - build tools/texcomp BC7 CLI (build/texcomp/texcomp)"
	@echo "make texcomp-c11-gate - strict C11 -Werror compile of texcomp"
	@echo "make texcomp-test - run texcomp unit tests (ASan+UBSan)"
	@echo "make texcomp-bench - run texcomp BC7 throughput benchmark"
	@echo "make texcomp-astc-psnr - ASTC encode/reference-decode PSNR table"
	@echo "make texcomp-arm - texcomp CLI with the vendored Arm astcenc backend (--encoder arm)"
	@echo "make texcomp-astc-arm-smoke - decode our ASTC output with Arm astcenc-native"
	@echo "make texcomp-astc-arm-gate - self-contained astcenc build + PSNR cross-check (CI gate)"
	@echo "make texcomp-basis-gate  - Basis Universal transcoder validation (cp basisu_transcoder to deps/basisu/)"
	@echo "make texcomp-wasm - Emscripten texcomp C API + Node CLI (scalar wasm)"
	@echo "make texcomp-wasm-simd - Emscripten texcomp C API + Node CLI (-msimd128)"
	@echo "make bench-compare - tinyexr-vs-OpenEXR codec comparison (needs OpenEXR build)"
	@echo "make fuzz   - build libFuzzer target (build/fuzz_v3)"
	@echo "make fuzz-corpus - replay regression corpus under ASan+UBSan+LSan"
	@echo "make fuzz-corpus-asan - replay corpus with LSan disabled for ptrace sandboxes"
	@echo "make parse-test - parse/load openexr-images corpus, classify PASS/XFAIL/FAIL"
	@echo "make examples-c - build native v3 example (build/exrinfo)"
	@echo "make wasm    - Emscripten WASM build of the v3 C API (build/exr_v3.mjs)"
	@echo "make freestanding-gate - prove the core builds with no libc (stdint/stddef only)"
	@echo "make arm-smoke - cross-build (aarch64) + run NEON SIMD smoke under qemu"
	@echo "make host-smoke - build + run the SIMD smoke test natively"
	@echo "make resize-test - tools/resize (tir) unit tests, ASan+UBSan"
	@echo "make resize-bench - tir throughput vs exr_resize_float (STB=1 adds stb2)"
	@echo "make resize-cli - build/tir_resize EXR resize tool (see tools/resize/README.md)"
	@echo ""
	@echo "DEFLATE=auto|libdeflate|intree selects the ZIP/ZIPS/PXR24 zlib backend"
	@echo "  (default: auto = vendored libdeflate, faster on natural images; both"
	@echo "  codecs link, switch at runtime via exr_zlib_set_backend). intree =="
	@echo "  in-tree pure-C only. Run 'make clean' when changing. LIBDEFLATE=1 is a"
	@echo "  legacy alias for DEFLATE=libdeflate. freestanding/wasm stay in-tree."
	@echo "Add THREADS=1 to any target for C11-threads parallel encode/decode"
	@echo "  (default: single-threaded). Set count via exr_set_num_threads()."
