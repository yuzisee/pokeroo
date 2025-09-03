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

cp -v consoleseparate.py/* ~/pokeroo-run/lib/
cp -v -p deploy/NewGame.Python.py ~/pokeroo-run/
```

### Play a game

```sh
cd ~/pokeroo-run
python3 NewGame.Python.py
```


# Troubleshooting

### Regenerate the opening book

Edit `src/debug_flags.h` to set SUPERPROGRESSUPDATE
Trigger any codepath that will invoke `PreflopCallStats::AutoPopulate()`
