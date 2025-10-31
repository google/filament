#!/usr/bin/env bash
owner=webmproject
project=libwebp
tag=v1.6.0
# ^^^ config / vvv github fetch repo tag archive and update boilerplate
tp_dir=$(_dir=$(dirname "${0}"); cd "${_dir}" && cd .. && cd .. && pwd)
cd "${tp_dir}" || exit 1
curl -L -O https://github.com/${owner}/${project}/archive/refs/tags/${tag}.zip || exit 1
unzip ${tag}.zip
mv ${project}-* ${project}_new
rsync -r ${project}_new/ ${project}/ --delete --exclude tnt
rm -rf ${tag}.zip ${project}_new
git add ${project} && git status
