TODO(from joseph): Continue https://github.com/yuzisee/pokeroo/commit/d964729929ee4495a762d8f929767029b0954d5a

# Getting started
```sh
cd holdem
make
```
* creates executable bin/holdem.arm64 on macOS
* creates executable bin/holdem.exe on Windows?
* creates executable e.g. bin/holdem.x86_64 on Linux

## Pre-calulate the database
```sh
mkdir -vp ~/pokeroo-run/lib
cd holdem
unzip "$(make echoreleasearchive | tail -n 1)" -d ~/pokeroo-run/lib/holdemdb/ # or, unzip a specific archive such as holdemdb_clang.zip, OR regenerate the files fresh (see `regenerate_opening_book` below)

cd "$(git rev-parse --show-toplevel)"
touch ~/pokeroo-run/lib/__init__.py
cp -v consoleseparate.web/* ~/pokeroo-run/lib/
cp -v -p deploy/NewGame.Python.py ~/pokeroo-run/
```

### Play a game

```sh
cd ~/pokeroo-run
python3 NewGame.Python.py
```


# Troubleshooting

### Run unit tests
```sh
cd holdem
make test
```

### Do a quick runtime profiling
e.g.
```sh
rm -r /tmp/profiling_dust.perf /tmp/profiling_dust.gcc /tmp/profiling_dust.x
mkdir -vp /tmp/profiling_dust.perf /tmp/profiling_dust.gcc /tmp/profiling_dust.x
cd holdem
GITHUB_ACTIONS="true" make db
# -4847 is "Ace-King offsuit" but you can choose any other hand if you prefer
HOLDEMDB_PATH="/tmp/profiling_dust.perf" perf record -F max -g -o /tmp/profiling_dust.perf/regenerate_opening_book.perf -- bin/regenerate_opening_book_selftest -4847
HOLDEMDB_PATH="/tmp/profiling_dust.gcc" bin/regenerate_opening_book_profiling -4847
HOLDEMDB_PATH="/tmp/profiling_dust.x" xcrun xctrace record --template 'CPU Profiler' --output /tmp/profiling_dust.x/regenerate_opening_book.trace --target-stdout - --launch -- bin/regenerate_opening_book-clang -4847

cd /tmp/profiling_dust.perf
perf report regenerate_opening_book.perf

# Linux only:
gprof bin/regenerate_opening_book_profiling bin/gmon*.out

# macOS only:
open /tmp/profiling_dust.x/regenerate_opening_book.trace

```

### Regenerate the opening book

Edit `src/debug_flags.h` to set SUPERPROGRESSUPDATE if needed
```sh
cd holdem
make db
```
Or, even better use the `int mode` multi-threading tools provided by `regenerateDb`
- `HOLDEMDB_PATH="/tmp/opening_book" bin/regenerate_opening_book 2`
- `HOLDEMDB_PATH="/tmp/opening_book" bin/regenerate_opening_book 3`

vs.

- `HOLDEMDB_PATH="/tmp/opening_book" bin/regenerate_opening_book 4`
- `HOLDEMDB_PATH="/tmp/opening_book" bin/regenerate_opening_book 5`
- `HOLDEMDB_PATH="/tmp/opening_book" bin/regenerate_opening_book 6`

vs.

- `HOLDEMDB_PATH="/tmp/opening_book" bin/regenerate_opening_book 7`
- `HOLDEMDB_PATH="/tmp/opening_book" bin/regenerate_opening_book 8`
- `HOLDEMDB_PATH="/tmp/opening_book" bin/regenerate_opening_book 9`
- `HOLDEMDB_PATH="/tmp/opening_book" bin/regenerate_opening_book 10`

etc.
