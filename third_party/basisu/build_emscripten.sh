#!/bin/sh
if ! emcc --version
then
	echo "Emscripten not found on PATH. Please follow instructions on https://emscripten.org/docs/getting_started/downloads.html"
	exit
fi

emcmake cmake .
emcmake make
