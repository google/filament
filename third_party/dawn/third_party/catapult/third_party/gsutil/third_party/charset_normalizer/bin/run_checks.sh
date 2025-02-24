#!/bin/sh -e

export PREFIX=""
if [ -d 'venv' ] ; then
    export PREFIX="venv/bin/"
fi

set -x

${PREFIX}pytest
${PREFIX}black --check --diff --target-version=py35 charset_normalizer
${PREFIX}flake8 charset_normalizer
${PREFIX}mypy charset_normalizer
${PREFIX}isort --check --diff charset_normalizer
