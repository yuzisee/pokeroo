TODO(from joseph): Run unit tests once, then clean warnings

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

### Regenerate the opening book

Edit `src/debug_flags.h` to set SUPERPROGRESSUPDATE if needed
```sh
make db
```
TODO(from joseph): Use the `int mode` mult-threading tools provided by `regenerateDb`
