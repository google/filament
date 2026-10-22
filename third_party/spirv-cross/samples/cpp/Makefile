# Copyright 2016-2021 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

SOURCES := $(wildcard *.comp)
SPIRV := $(SOURCES:.comp=.spv)
CPP_INTERFACE := $(SOURCES:.comp=.spv.cpp)
CPP_DRIVER := $(SOURCES:.comp=.cpp)
EXECUTABLES := $(SOURCES:.comp=.shader)
OBJECTS := $(CPP_DRIVER:.cpp=.o) $(CPP_INTERFACE:.cpp=.o)

CXXFLAGS += -std=c++11 -I../../include -I.
LDFLAGS += -pthread -lm

all: $(EXECUTABLES)

%.spv: %.comp
	glslangValidator -V -o $@ $<

%.spv.cpp: %.spv
	../../spirv-cross --cpp --output $@ $<

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

%.shader: %.o %.spv.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) -f $(EXECUTABLES) $(SPIRV) $(CPP_INTERFACE) $(OBJECTS)

.PHONY: clean
