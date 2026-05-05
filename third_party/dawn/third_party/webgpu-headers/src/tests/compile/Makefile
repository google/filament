# Makefile for GNU make

# Always rebuild all targets.
.PHONY: all clean main-c.out main-cpp.out

all: main-c.out main-cpp.out

clean:
	rm -f main-c.out main-cpp.out

# Enable all but a few warnings, then also enable a few extras that clang on Mac
# forces off by default even with -Weverything
CLANG_WARNINGS = -Werror -Weverything \
	-Wno-unused-variable \
	-Wno-padded -Wno-poison-system-directories -Wno-c++98-compat-pedantic \
	-Wc99-designator \
	-Welaborated-enum-base \
	-Wenum-enum-conversion \
	-Wenum-float-conversion \
	-Wextra-semi-stmt \
	-Wfinal-dtor-non-final-class \
	-Wgnu-folding-constant \
	-Wimplicit-fallthrough \
	-Wimplicit-int-float-conversion \
	-Wmisleading-indentation \
	-Wquoted-include-in-framework-header \
	-Wreorder-init-list \
	-Wreserved-identifier \
	# phew.
GCC_WARNINGS   = -Werror -Wall -Wextra \
	-Wno-unused-variable -Wno-unused-but-set-variable

main-c.out: main.c main.inl init_tests_autogen.inl ../../webgpu.h Makefile
	# Compile as C
	clang -o $@ $< -std=c99 $(CLANG_WARNINGS)
	gcc   -o $@ $< -std=c99 $(GCC_WARNINGS)
	clang -o $@ $< -std=c11 $(CLANG_WARNINGS)
	gcc   -o $@ $< -std=c11 $(GCC_WARNINGS)
	clang -o $@ $< -std=c17 $(CLANG_WARNINGS)
	gcc   -o $@ $< -std=c17 $(GCC_WARNINGS)
	# Pre-release C23
	clang -o $@ $< -std=c2x $(CLANG_WARNINGS)
	gcc   -o $@ $< -std=c2x $(GCC_WARNINGS)

main-cpp.out: main.cpp main.inl init_tests_autogen.inl ../../webgpu.h Makefile
	# Compile as C++
	clang++ -o $@ $< -std=c++98 $(CLANG_WARNINGS) -Wno-c++11-long-long
	g++     -o $@ $< -std=c++98 $(GCC_WARNINGS)
	clang++ -o $@ $< -std=c++03 $(CLANG_WARNINGS) -Wno-c++11-long-long
	g++     -o $@ $< -std=c++03 $(GCC_WARNINGS)
	clang++ -o $@ $< -std=c++11 $(CLANG_WARNINGS)
	g++     -o $@ $< -std=c++11 $(GCC_WARNINGS)
	clang++ -o $@ $< -std=c++14 $(CLANG_WARNINGS)
	g++     -o $@ $< -std=c++14 $(GCC_WARNINGS)
	clang++ -o $@ $< -std=c++17 $(CLANG_WARNINGS)
	g++     -o $@ $< -std=c++17 $(GCC_WARNINGS)
	clang++ -o $@ $< -std=c++20 $(CLANG_WARNINGS)
	g++     -o $@ $< -std=c++20 $(GCC_WARNINGS)
	# Pre-release C++23
	clang++ -o $@ $< -std=c++2b $(CLANG_WARNINGS)
	g++     -o $@ $< -std=c++2b $(GCC_WARNINGS)
