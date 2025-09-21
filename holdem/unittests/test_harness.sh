#!/bin/sh
#
# Example usage:
# cd unittests && sh test_harness.sh ../bin/unittest_clang

set -u

set -e

set -x

# Clean up any old files, if they exist
mkdir -p playlogs.old
echo 'CLEANUP start'
ls -1 playlogs.expected/ | xargs -I @ sh -c "test '!' -f '@' || mv -v '@' 'playlogs.old/@'"
# ^^^ there's no `set -o pipefail` (it's not portable anyway) so this succeeds even if there is nothing to move

# Set up the playlogs comparison folder for actual output
mkdir -p playlogs.actual
find playlogs.actual/ -iname '*.txt' -exec rm -v '{}' '+'

# Run the command given!
# e.g. if you invoked `sh test_harness.sh echo hello world` the line below will print "hello world" to the console
"$@"

# Grab the "actual" playlogs file corresponding to each "expected" playlogs file
cd playlogs.expected/
# ls -1 playlogs.expected/ | xargs -I @ mv -v @ playlogs.actual/@
find . -iname '*.txt' -exec mv -v ../'{}' ../playlogs.actual/ ';'
# ^^^ The `ls -1 … | xargs` version is simpler, but we need this script to be strict, even without `set -o pipefail` so this is the best we can come up with.
cd ..

# COMPARE!
if test -z "$(ls -A playlogs.expected/)"; then
  find playlogs.expected
  echo 'Where are the reference logs?' 1>&2
  pwd -P
  ls -la
  find playlogs.actual
  exit 66 # 65 would be EX_DATAERR, 78 would be EX_CONFIG. We went with 66 EX_NOINPUT
elif diff -urw playlogs.expected/ playlogs.actual/; then
  echo 'PLAYLOGS MATCH ✅'
else
  DIFF_CMD_EXIT_CODE=$?
  echo 'System test failed during: ' "$*" 1>&2
  exit $DIFF_CMD_EXIT_CODE
fi
