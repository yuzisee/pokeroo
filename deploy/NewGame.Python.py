#!/usr/bin/env python

# To deploy, put this script in a directory containing:
#   NewGame.Python.py (this script)
#   lib/ (directory)
#   lib/holdemdb/ (directory containing 22x.holdemC, ... , TTx.holdemW, etc.)
#   lib/__init__.py (empty file)
#   lib/consoleseparate.py (python file implementing ConsoleSeparate)
#

import datetime
import os
import os.path
import logging
import time
import sys

import lib.consoleseparate

def main(PLAYERNAME):
    script_dir = os.path.realpath(os.path.dirname(__file__))

    # http://stackoverflow.com/questions/4548684/getting-the-time-since-the-epoch
    unix_timestamp = time.time()
    localtime_struct = datetime.datetime.fromtimestamp(unix_timestamp)
    localtime_suffix = '{0.year}.{0.month:02d}.{0.day:02d}-{0.hour}.{0.minute}.{0.second}'.format(localtime_struct)

    # Create the target files
    target_dir = os.path.join(script_dir, 'Results.{0}'.format(localtime_suffix))
    continue_script = os.path.join(script_dir, 'ContinueGame_{0}.py'.format(localtime_suffix))
    os.makedirs(os.path.join(target_dir, 'saves'))
    holdem_env = {'HOLDEMDB_PATH': os.path.join(script_dir, 'lib', 'holdemdb')}
    holdem_cmd = os.path.join(script_dir, 'lib', 'holdem')
    gamelog_file = os.path.join(target_dir, 'game.log')

    continue_str = """#!/usr/bin/env python3

import lib.consoleseparate

holdem_env = dict([])
holdem_env['HOLDEMDB_PATH'] = "{holdemdb_path}"

with open("{gamelog_file}", 'a') as f:
    lib.consoleseparate.run_cmd("{holdem_cmd}", "{target_dir}", holdem_env, f.write)

""".format(holdemdb_path=holdem_env['HOLDEMDB_PATH'],
           gamelog_file=gamelog_file,
           holdem_cmd=holdem_cmd,
           target_dir=target_dir )


    with open(continue_script, 'a') as f:
        f.write(continue_str)

    with open(gamelog_file, 'a') as f:
        lib.consoleseparate.run_cmd([holdem_cmd, PLAYERNAME], target_dir, holdem_env, f.write)


if __name__ == '__main__':
    main('You')
