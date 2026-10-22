This folder was created as follows:

curl -L -O https://github.com/aras-p/smol-v/archive/9a787d1.zip
unzip 9a787d1.zip
mv smol-v-* smol-v
cd smol-v
rm -rf tests projects testing
mv license.txt LICENSE

Local modifications:

source/smolv.cpp is not a pristine copy of the upstream archive. The local changes are kept as
patch files in this directory; re-applying the recipe above drops them, so apply them again from
the third_party directory:

patch -p2 < smol-v/tnt/0001-decode-output-bounds-checks.patch

- 0001-decode-output-bounds-checks.patch
  Output bounds checks in smolv::Decode (b/557280759). The decoder wrote decoded SPIR-V words
  into the caller's buffer without using its size as a write bound, so a blob declaring a decoded
  size smaller than it actually decodes to overflowed the heap allocation.
  Hand-ported from the unmerged upstream PR https://github.com/aras-p/smol-v/pull/16.
  Drop this patch once that PR is merged and smol-v is updated past it.
