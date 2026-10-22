#!/bin/sh
rm -rf assimp/
mkdir assimp
swig -c++ -d -outcurrentdir -I../../../include -splitproxy -package assimp $@ ../assimp.i
