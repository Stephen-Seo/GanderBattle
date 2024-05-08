#!/usr/bin/env sh

pushd "$(dirname "$0")" >&/dev/null
PATH_TO_SCRIPT="$(pwd)"
popd >&/dev/null

find "${PATH_TO_SCRIPT}/src" -regex '.*\.\(h\|cc\)$' -exec echo 'Formatting {}...' ';' -execdir clang-format -i --style=Google '{}' ';'
