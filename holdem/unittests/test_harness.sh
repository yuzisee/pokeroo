#!/bin/sh
#
# Example usage:
# cd unittests && sh test_harness.sh ../bin/unittest_clang

set -u

set -e
set -o pipefail

set -x

# Clean up any old files, if they exist
mkdir -p playlogs.old
ls -1 playlogs.expected/ | xargs -I @ mv @ playlogs.old/@

# Set up the playlogs comparison folder for actual output
mkdir -p playlogs.actual
find playlogs.actual/ -iname '*.txt' -exec rm -v '{}' '+'

# Run the command given!
# e.g. if you invoked `sh test_harness.sh echo hello world` the line below will print "hello world" to the console
"$@"

# Grab the "actual" playlogs file corresponding to each "expected" playlogs file
ls -1 playlogs.expected/ | xargs -I @ mv @ playlogs.actual/@

# COMPARE!
if diff -urw playlogs.expected/ playlogs.actual/; then
  echo 'PLAYLOGS MATCH ✅'
else
  DIFF_CMD_EXIT_CODE=$?
  echo 'System test failed during: ' "$*"
  exit $DIFF_CMD_EXIT_CODE
fi
