#!/bin/sh

set +x # Do not print

# https://gist.github.com/mohanpedala/1e2ff5661761d3abd0385e8223e16425
set -u

if command -v nproc 1> /dev/null; then
  NTHREADS=$(nproc)
elif command -v sysctl 1> /dev/null; then
  NTHREADS=$(sysctl -n hw.logicalcpu)
else
  NTHREADS=1
fi

if test '1' -lt "${NTHREADS}"; then
  # Leave one thread open for the Github Actions runner itself
  NTHREADS=$(expr "$NTHREADS" '-' 1)
fi

echo "$NTHREADS"
