#!/bin/bash

#usage example 
#   dumpSpv vshader_vert.spv vshader

set -e 
shader_file_in=$1
shader_file_out=$2
shader_id="${shader_file_out}"
out="${shader_file_out}.h"
pushd ./
echo $out
echo "#pragma once" > $out
chmod +w $out
echo "static uint32_t spv_${shader_id}[] = " >> $out
echo "{" >> $out
hexdump -v -e '/4 "    %#08x,\n"' $shader_file_in >> $out
echo "};" >> $out
popd

