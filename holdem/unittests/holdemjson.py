#!/usr/bin/env python3
"""Convert between .holdemC/.holdemW binary format ↔ JSON format"""

import json
import struct
import sys
import typing

COARSE_COMMUNITY_NUM_BINS = 7
SIZEOF_CACHESIZE_T = 4 # uint32 is 4 bytes

class StatResult(typing.TypedDict):
    wins: float
    splits: float
    loss: float
    repeated: float
    pct: float

def read_typed_float(f: typing.IO) -> float:
  # https://docs.python.org/3/library/struct.html#format-characters
  return struct.unpack('d', f.read(8))[0]

def statresult_to_dict(f: typing.IO) -> StatResult:
    """StatsManager::serializeStatResult⁻¹ ∘ StatsManager::StatResultToJSON"""
    statresult: StatResult = {
        "wins": read_typed_float(f),
        "splits": read_typed_float(f),
        "loss": read_typed_float(f),
        "repeated": read_typed_float(f),
        "pct": read_typed_float(f)
    }
    return statresult

def holdem_c_to_json(holdemc_filename: str) -> str:
    """StatsManager::SerializeC⁻¹ ∘ StatsManager::holdemCtoJSON"""
    holdem_c_json = dict()
    with open(holdemc_filename, 'rb') as f:
        holdem_c_json["!vcount"] = struct.unpack('I', f.read(SIZEOF_CACHESIZE_T))[0]
        holdem_c_json["cumulation"] = list()
        for _ in range(holdem_c_json["!vcount"]):
            holdem_c_json["cumulation"].append(statresult_to_dict(f))

        no_leftover = f.read(1)
        assert no_leftover == b'', f"Corrupt {holdemc_filename} ⚠ CallCumulation contains at least {repr(no_leftover)} after deserializing...\n{holdem_c_json}"

    # [!NOTE]
    # There's a bit of wonkiness here to match the whitespace/indentation/formatting of StatsManager::holdemCtoJSON on the C++ side
    return json.dumps(holdem_c_json, sort_keys=False).replace('{', "\n    {").replace('"!vcount"', "\n" + '  "!vcount"').replace('"cumulation"', "\n" + '  "cumulation"').replace(']', "\n" + '  ]').replace('.0,', ',').replace('.0}', '}').replace('.0}', '}').strip()[:-1] + "\n" + '}'

def holdem_w_to_json(holdemc_filename: str) -> str:
    """StatsManager::serializeDistrShape⁻¹ ∘ StatsManager::holdemWtoJSON"""
    holdem_w_json = dict()
    with open(holdemc_filename, 'rb') as f:
        holdem_w_json["n"] = read_typed_float(f)
        holdem_w_json["mean"] = statresult_to_dict(f)
        holdem_w_json["best"] = statresult_to_dict(f)
        holdem_w_json["worst"] = statresult_to_dict(f)
        holdem_w_json["coarseHistogram"] = [statresult_to_dict(f) for _ in range(COARSE_COMMUNITY_NUM_BINS)]
        holdem_w_json["avgDev"] = read_typed_float(f)
        holdem_w_json["stdDev"] = read_typed_float(f)
        holdem_w_json["improve"] = read_typed_float(f)
        holdem_w_json["skew"] = read_typed_float(f)
        holdem_w_json["kurtosis"] = read_typed_float(f)

        no_leftover = f.read(1)
        assert no_leftover == b'', f"Corrupt {holdemc_filename} ⚠ DistrShape contains at least {repr(no_leftover)} after deserializing…\n{holdem_w_json}"

    # [!NOTE]
    # Please don't mind the bit of wonkiness here as we try to match the exact whitespace/indentation/formatting of StatsManager::holdemCtoJSON on the C++ side
    json_to_format = json.dumps(holdem_w_json, sort_keys=False)
    for toplevel_k in holdem_w_json.keys():
        json_to_format = json_to_format.replace('"' + toplevel_k + '"', "\n" + '  "' + toplevel_k + '"')
    return json_to_format.replace('[', '[' + "\n    ").replace('}, {', '},' + "\n" + '    {').replace(']', "\n" + '  ]').strip()[:-1] + "\n" + '}'

def usage():
    sys.stderr.write("\n")
    sys.stderr.write("USAGE:\n")
    sys.stderr.write('  $0 Q7x.holdemC # will print to the screen as JSON')
    sys.stderr.write("\n")
    sys.stderr.write('  $0 Q7x.holdemW # will print to the screen as JSON')
    sys.stderr.write("\n")
    # sys.stderr.write(r"  $0 Q7x.holdemC.json # will read JSON and write Q7x.holdemC if doesn't already exist")
    # sys.stderr.write("\n")
    # sys.stderr.write(r"  $0 Q7x.holdemW.json # will read JSON and write Q7x.holdemW if doesn't already exist")
    # sys.stderr.write("\n")

def main(argv):
    assert sys.byteorder == 'little', "See src/aiCache.cpp:abort_if_big_endian ← we *can* make this work, but you have to be _very_ careful"

    if len(argv) == 2:
        if argv[1].endswith('.holdemC'):
            print(holdem_c_to_json(argv[1]))
            return
        if argv[1].endswith('.holdemW'):
            print(holdem_w_to_json(argv[1]))
            return
#         if argv[0].endswith('.holdem.jsonC'):
#             save_holdem_c(argv[0])
#             return
#         if argv[0].endswith('.holdem.jsonW'):
#             save_holdem_w(argv[0])
#             return

    sys.stderr.write(repr(argv))
    usage()
    # man sysexits
    sys.exit(64) # EX_USAGE

if __name__ == '__main__':
    main(sys.argv)
