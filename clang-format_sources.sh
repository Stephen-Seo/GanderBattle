#!/usr/bin/env sh

find "$(pwd)/src" -regex '.*\.\(h\|cc\)$' -exec echo 'Formatting {}...' ';' -execdir clang-format -i --style=Google '{}' ';'
