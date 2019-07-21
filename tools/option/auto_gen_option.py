#!/usr/bin/env python3

import sys
import os

from base_type import BaseType
from gen_data import GenData


cur_dir = os.path.dirname(os.path.abspath(__file__))


def get_file_data(filename):
    with open(os.path.join(cur_dir, filename), 'r') as f:
        return f.read()
    return None


def save_result(filename, data):
    with open(filename, "w") as f:
        f.write(data)


def generate_output_file(gtype, outfile):
    gd = GenData()
    for k in gtype:
        output = None
        if k == 'c':
            template_file = get_file_data("option.c.in")
            output = template_file % gd.codec
        elif k == 'h':
            template_file = get_file_data("option.h.in")
            output = template_file % gd.codeh
        elif k == 'e':
            output = gd.codef
        else:
            assert(False)
        if output is not None:
            save_result(outfile, output)


def print_usage():
    print('usage: ./auto_gen_option_c.py [c|h|e] [output]')
    sys.exit(-1)


def check_base_type():
    for k in BaseType['type']:
        if k in BaseType['base_type']:
            return False
    for k in BaseType['data']:
        t = BaseType['data'][k]['type']
        if t not in BaseType['type'] and t not in BaseType['base_type']:
            return False
    return True


def main(argv):
    if (len(argv) != 2):
        print_usage()

    assert(check_base_type())
    assert(os.access(os.path.dirname(argv[1]), os.W_OK))
    assert(not os.access(argv[1], os.F_OK) or os.access(argv[1], os.W_OK))
    generate_output_file(argv[0], argv[1])


if __name__ == '__main__':
    main(sys.argv[1:])
