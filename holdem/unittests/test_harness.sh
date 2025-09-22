#!/bin/sh
#
# Example usage:
# cd unittests && sh test_harness.sh ../bin/unittest_clang

set -u
set -e

# Clean up any old files, if they exist
mkdir -p playlogs.old
echo 'CLEANUP start'
ls -1 playlogs.expected/ | xargs -I @ sh -c "test '!' -f '@' || mv -v '@' 'playlogs.old/@'"
# ^^^ there's no `set -o pipefail` (it's not portable anyway) so this succeeds even if there is nothing to move

set -x

# Set up the playlogs comparison folder for actual output
mkdir -p playlogs.actual
find playlogs.actual/ -iname '*.txt' -exec rm -v '{}' '+'

# Run the command given!
# e.g. if you invoked `sh test_harness.sh echo hello world` the line below will print "hello world" to the console
"$@"

# If running as part of Github Actions, also make a downloadable archive of the results
if test "${GITHUB_ACTIONS+y}" '=' 'y'; then
  # INVARIANT: If you get here, $GITHUB_ACTIONS is defined. (Since we're using `set -u` above, we need to perform this check before trying to read the value of $GITHUB_ACTIONS)
  if test "$GITHUB_ACTIONS" '=' 'true'; then
    # INVARIANT: If you get here, $GITHUB_ACTIONS is defined AND set to "true". (This is the value you'll have inside any Github Actions runner that is executing a Github Actions job)
    tar -cv *.txt | xz -vv -z -9 -e > "../playlogs.$(basename $1)-${RUNNER_NAME}-$(hostname)_${GITHUB_SHA}.${GITHUB_HEAD_REF}_unittests.tar.xz"
  fi
fi

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
elif diff -U 8 -rw playlogs.expected/ playlogs.actual/; then
  echo 'PLAYLOGS MATCH ✅'
else
  DIFF_CMD_EXIT_CODE=$?
  echo 'System test failed during: ' "$*" 1>&2
  exit $DIFF_CMD_EXIT_CODE
fi
