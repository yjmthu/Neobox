#!/usr/bin/env python3

"""This script splits httplib.h into .h and .cc parts."""

import argparse
import os
import sys

border = '// ----------------------------------------------------------------------------'

cur_dir = os.path.dirname(sys.argv[0])
lib_name = 'httplib'
source_name = './src/' + lib_name + '.' + '.cpp'
# get the input file
in_file = './' + lib_name + '.h'
# get the output file
h_out = './include/' + lib_name + '.h'
cc_out = './src/' + lib_name + '.' + '.cpp'

# if the modification time of the out file is after the in file,
# don't split (as it is already finished)
do_split = True

if os.path.exists(h_out):
    in_time = os.path.getmtime(in_file)
    out_time = os.path.getmtime(h_out)
    do_split = in_time > out_time

if do_split:
    with open(in_file) as f:
        lines = f.readlines()

    python_version = sys.version_info[0]

    in_implementation = False
    with open(h_out, 'w') as fh, open(cc_out, 'w') as fc:
        fc.write('#include "httplib.h"\n')
        fc.write('namespace httplib {\n')
        for line in lines:
            is_border_line = border in line
            if is_border_line:
                in_implementation = not in_implementation
            elif in_implementation:
                fc.write(line.replace('inline ', ''))
            else:
                fh.write(line)
        fc.write('} // namespace httplib\n')

    print("Wrote {} and {}".format(h_out, cc_out))
else:
    print("{} and {} are up to date".format(h_out, cc_out))
