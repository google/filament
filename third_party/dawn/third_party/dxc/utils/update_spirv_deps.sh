#!/bin/bash
if [ ! -d external -o ! -d .git -o ! -d azure-pipelines ] ; then
  echo "Run this script on the top-level directory of the DXC repository."
  exit 1
fi

# Ignore effcee, re2 and DirectX-Headers updates.  See the discussion at
# https://github.com/microsoft/DirectXShaderCompiler/pull/5246
# for details.
git submodule foreach '                                                   \
  n=$(basename $sm_path);                                                 \
  if [ "$n" != "re2" -a "$n" != "effcee" -a "$n" != "DirectX-Headers" ];  \
  then git switch main && git pull --ff-only;                             \
  else echo "Skipping submodule $n";                                      \
  fi;                                                                     \
  echo                                                                    \
  '
git add external
exit 0
