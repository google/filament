MAKEFLAGS+=-r -j

config=debug
files=demo/pirate.obj

BUILD=build/$(config)

LIBRARY_SOURCES=$(wildcard src/*.cpp)
LIBRARY_OBJECTS=$(LIBRARY_SOURCES:%=$(BUILD)/%.o)

DEMO_SOURCES=$(wildcard demo/*.c demo/*.cpp) tools/meshloader.cpp
DEMO_OBJECTS=$(DEMO_SOURCES:%=$(BUILD)/%.o)

GLTFPACK_SOURCES=$(wildcard gltf/*.cpp) tools/meshloader.cpp
GLTFPACK_OBJECTS=$(GLTFPACK_SOURCES:%=$(BUILD)/%.o)

OBJECTS=$(LIBRARY_OBJECTS) $(DEMO_OBJECTS) $(GLTFPACK_OBJECTS)

LIBRARY=$(BUILD)/libmeshoptimizer.a
DEMO=$(BUILD)/meshoptimizer

CFLAGS=-g -Wall -Wextra -Werror -std=c89
CXXFLAGS=-g -Wall -Wextra -Wshadow -Wno-missing-field-initializers -Werror -std=c++98
LDFLAGS=

$(GLTFPACK_OBJECTS): CXXFLAGS+=-std=c++11

ifdef BASISU
    $(GLTFPACK_OBJECTS): CXXFLAGS+=-DWITH_BASISU
    $(BUILD)/gltf/basis%.cpp.o: CXXFLAGS+=-I$(BASISU)
    gltfpack: LDFLAGS+=-lpthread

    ifeq ($(HOSTTYPE),x86_64)
        $(BUILD)/gltf/basislib.cpp.o: CXXFLAGS+=-msse4.1
    endif
endif

WASMCC?=$(WASI_SDK)/bin/clang++
WASIROOT?=$(WASI_SDK)/share/wasi-sysroot

WASM_FLAGS=--target=wasm32-wasi --sysroot=$(WASIROOT)
WASM_FLAGS+=-O3 -DNDEBUG -nostartfiles -nostdlib -Wl,--no-entry -Wl,-s
WASM_FLAGS+=-fno-slp-vectorize -fno-vectorize -fno-unroll-loops
WASM_FLAGS+=-Wl,-z -Wl,stack-size=24576 -Wl,--initial-memory=65536
WASM_EXPORT_PREFIX=-Wl,--export

WASM_DECODER_SOURCES=src/vertexcodec.cpp src/indexcodec.cpp src/vertexfilter.cpp tools/wasmstubs.cpp
WASM_DECODER_EXPORTS=meshopt_decodeVertexBuffer meshopt_decodeIndexBuffer meshopt_decodeIndexSequence meshopt_decodeFilterOct meshopt_decodeFilterQuat meshopt_decodeFilterExp sbrk __wasm_call_ctors

WASM_ENCODER_SOURCES=src/vertexcodec.cpp src/indexcodec.cpp src/vertexfilter.cpp src/vcacheoptimizer.cpp src/vfetchoptimizer.cpp tools/wasmstubs.cpp
WASM_ENCODER_EXPORTS=meshopt_encodeVertexBuffer meshopt_encodeVertexBufferBound meshopt_encodeIndexBuffer meshopt_encodeIndexBufferBound meshopt_encodeIndexSequence meshopt_encodeIndexSequenceBound meshopt_encodeVertexVersion meshopt_encodeIndexVersion meshopt_encodeFilterOct meshopt_encodeFilterQuat meshopt_encodeFilterExp meshopt_optimizeVertexCache meshopt_optimizeVertexCacheStrip meshopt_optimizeVertexFetchRemap sbrk __wasm_call_ctors

WASM_SIMPLIFIER_SOURCES=src/simplifier.cpp src/vfetchoptimizer.cpp tools/wasmstubs.cpp
WASM_SIMPLIFIER_EXPORTS=meshopt_simplify meshopt_simplifyScale meshopt_optimizeVertexFetchRemap sbrk __wasm_call_ctors

ifeq ($(config),iphone)
	IPHONESDK=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk
	CFLAGS+=-arch armv7 -arch arm64 -isysroot $(IPHONESDK)
	CXXFLAGS+=-arch armv7 -arch arm64 -isysroot $(IPHONESDK) -stdlib=libc++
	LDFLAGS+=-arch armv7 -arch arm64 -isysroot $(IPHONESDK) -L $(IPHONESDK)/usr/lib -mios-version-min=7.0
endif

ifeq ($(config),trace)
	CXXFLAGS+=-DTRACE=1
endif

ifeq ($(config),scalar)
	CXXFLAGS+=-O3 -DNDEBUG -DMESHOPTIMIZER_NO_SIMD
endif

ifeq ($(config),release)
	CXXFLAGS+=-O3 -DNDEBUG
endif

ifeq ($(config),coverage)
	CXXFLAGS+=-coverage
	LDFLAGS+=-coverage
endif

ifeq ($(config),sanitize)
	CXXFLAGS+=-fsanitize=address,undefined -fno-sanitize-recover=all
	LDFLAGS+=-fsanitize=address,undefined
endif

ifeq ($(config),analyze)
	CXXFLAGS+=--analyze
endif

all: $(DEMO)

test: $(DEMO)
	$(DEMO) $(files)

check: $(DEMO)
	$(DEMO)

dev: $(DEMO)
	$(DEMO) -d $(files)

format:
	clang-format -i $(LIBRARY_SOURCES) $(DEMO_SOURCES) $(GLTFPACK_SOURCES)

js: js/meshopt_decoder.js js/meshopt_decoder.module.js js/meshopt_encoder.js js/meshopt_encoder.module.js js/meshopt_simplifier.js js/meshopt_simplifier.module.js

gltfpack: $(BUILD)/gltfpack
	ln -fs $^ $@

$(BUILD)/gltfpack: $(GLTFPACK_OBJECTS) $(LIBRARY)
	$(CXX) $^ $(LDFLAGS) -o $@

gltfpack.wasm: gltf/library.wasm

gltf/library.wasm: ${LIBRARY_SOURCES} ${GLTFPACK_SOURCES} tools/meshloader.cpp
	$(WASMCC) $^ -o $@ -Os -DNDEBUG --target=wasm32-wasi --sysroot=$(WASIROOT) -nostartfiles -Wl,--no-entry -Wl,--export=pack -Wl,--export=malloc -Wl,--export=free -Wl,--export=__wasm_call_ctors -Wl,-s -Wl,--allow-undefined-file=gltf/wasistubs.txt

build/decoder_base.wasm: $(WASM_DECODER_SOURCES)
	@mkdir -p build
	$(WASMCC) $^ $(WASM_FLAGS) $(patsubst %,$(WASM_EXPORT_PREFIX)=%,$(WASM_DECODER_EXPORTS)) -o $@

build/decoder_simd.wasm: $(WASM_DECODER_SOURCES)
	@mkdir -p build
	$(WASMCC) $^ $(WASM_FLAGS) $(patsubst %,$(WASM_EXPORT_PREFIX)=%,$(WASM_DECODER_EXPORTS)) -o $@ -msimd128 -mbulk-memory

build/encoder.wasm: $(WASM_ENCODER_SOURCES)
	@mkdir -p build
	$(WASMCC) $^ $(WASM_FLAGS) $(patsubst %,$(WASM_EXPORT_PREFIX)=%,$(WASM_ENCODER_EXPORTS)) -lc -o $@

build/simplifier.wasm: $(WASM_SIMPLIFIER_SOURCES)
	@mkdir -p build
	$(WASMCC) $^ $(WASM_FLAGS) $(patsubst %,$(WASM_EXPORT_PREFIX)=%,$(WASM_SIMPLIFIER_EXPORTS)) -lc -o $@

js/meshopt_decoder.js: build/decoder_base.wasm build/decoder_simd.wasm tools/wasmpack.py
	sed -i "s#Built with clang.*#Built with $$($(WASMCC) --version | head -n 1 | sed 's/\s\+(.*//')#" $@
	sed -i "s#\(var wasm_base = \)\".*\";#\\1\"$$(cat build/decoder_base.wasm | python3 tools/wasmpack.py)\";#" $@
	sed -i "s#\(var wasm_simd = \)\".*\";#\\1\"$$(cat build/decoder_simd.wasm | python3 tools/wasmpack.py)\";#" $@

js/meshopt_encoder.js: build/encoder.wasm tools/wasmpack.py
	sed -i "s#Built with clang.*#Built with $$($(WASMCC) --version | head -n 1 | sed 's/\s\+(.*//')#" $@
	sed -i "s#\(var wasm = \)\".*\";#\\1\"$$(cat build/encoder.wasm | python3 tools/wasmpack.py)\";#" $@

js/meshopt_simplifier.js: build/simplifier.wasm tools/wasmpack.py
	sed -i "s#Built with clang.*#Built with $$($(WASMCC) --version | head -n 1 | sed 's/\s\+(.*//')#" $@
	sed -i "s#\(var wasm = \)\".*\";#\\1\"$$(cat build/simplifier.wasm | python3 tools/wasmpack.py)\";#" $@

js/%.module.js: js/%.js
	sed '/UMD-style export/,$$d' <$< >$@
	sed -n "s#\s*module.exports = \(.*\);#export { \\1 };#p" <$< >>$@

$(DEMO): $(DEMO_OBJECTS) $(LIBRARY)
	$(CXX) $^ $(LDFLAGS) -o $@

vcachetuner: tools/vcachetuner.cpp $(BUILD)/tools/meshloader.cpp.o $(BUILD)/demo/miniz.cpp.o $(LIBRARY)
	$(CXX) $^ -fopenmp $(CXXFLAGS) -std=c++11 $(LDFLAGS) -o $@

codecbench: tools/codecbench.cpp $(LIBRARY)
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -o $@

codecbench.js: tools/codecbench.cpp ${LIBRARY_SOURCES}
	emcc $^ -O3 -g -DNDEBUG -s TOTAL_MEMORY=268435456 -s SINGLE_FILE=1 -o $@

codecbench-simd.js: tools/codecbench.cpp ${LIBRARY_SOURCES}
	emcc $^ -O3 -g -DNDEBUG -s TOTAL_MEMORY=268435456 -s SINGLE_FILE=1 -msimd128 -o $@

codecbench.wasm: tools/codecbench.cpp ${LIBRARY_SOURCES}
	$(WASMCC) $^ -fno-exceptions --target=wasm32-wasi --sysroot=$(WASIROOT) -lc++ -lc++abi -O3 -g -DNDEBUG -o $@

codecbench-simd.wasm: tools/codecbench.cpp ${LIBRARY_SOURCES}
	$(WASMCC) $^ -fno-exceptions --target=wasm32-wasi --sysroot=$(WASIROOT) -lc++ -lc++abi -O3 -g -DNDEBUG -msimd128 -o $@

codecfuzz: tools/codecfuzz.cpp src/vertexcodec.cpp src/indexcodec.cpp
	$(CXX) $^ -fsanitize=fuzzer,address,undefined -O1 -g -o $@

$(LIBRARY): $(LIBRARY_OBJECTS)
	ar rcs $@ $^

$(BUILD)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $< $(CXXFLAGS) -c -MMD -MP -o $@

$(BUILD)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $< $(CFLAGS) -c -MMD -MP -o $@

-include $(OBJECTS:.o=.d)

clean:
	rm -rf $(BUILD)

.PHONY: all clean format js
