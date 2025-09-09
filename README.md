TODO(from joseph): Continue https://github.com/yuzisee/pokeroo/commit/d964729929ee4495a762d8f929767029b0954d5a

# Getting started
```sh
cd holdem
make
```
* creates executable bin/holdem.arm64 on macOS
* creates executable bin/holdem.exe on Windows?
* creates executable bin/holdem on Linux??

## Pre-calulate the database
```sh
cd "$(git rev-parse --show-toplevel)"

mkdir -p ~/pokeroo-run/lib
unzip holdem/holdemdb.zip -d ~/pokeroo-run/lib/holdemdb/ # or, regenerate them (see below)

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
make test
```

### Regenerate the opening book

Edit `src/debug_flags.h` to set SUPERPROGRESSUPDATE if needed
```sh
make db
```
Or, even better use the `int mode` mult-threading tools provided by `regenerateDb`
- `HOLDEMDB_PATH=/tmp/opening_book bin/regenerate_opening_book 2`
- `HOLDEMDB_PATH=/tmp/opening_book bin/regenerate_opening_book 3`

vs.

- `HOLDEMDB_PATH=/tmp/opening_book bin/regenerate_opening_book 4`
- `HOLDEMDB_PATH=/tmp/opening_book bin/regenerate_opening_book 5`
- `HOLDEMDB_PATH=/tmp/opening_book bin/regenerate_opening_book 6`

vs.

- `HOLDEMDB_PATH=/tmp/opening_book bin/regenerate_opening_book 7`
- `HOLDEMDB_PATH=/tmp/opening_book bin/regenerate_opening_book 8`
- `HOLDEMDB_PATH=/tmp/opening_book bin/regenerate_opening_book 9`
- `HOLDEMDB_PATH=/tmp/opening_book bin/regenerate_opening_book 10`

etc.
