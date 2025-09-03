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
mkdir -p ~/pokeroo-run
unzip holdem/holdemdb.zip -d ~/pokeroo-run/holdemdb/ # or, regenerate them (see below)

```
Either extract `holdemdb.zip`

### Play a game

Follow the instructions in `deploy`


# Troubleshooting

### Regenerate the opening book

Edit `src/debug_flags.h` to set SUPERPROGRESSUPDATE
Trigger any codepath that will invoke `PreflopCallStats::AutoPopulate()`
