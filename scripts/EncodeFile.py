#!/usr/bin/env python3
import os
import pathlib
import argparse
import jinja2
import fbp

 # [0,1,2] = reserved values
RESERVED_CODES = 3

def isDir(string):
    if os.path.isdir(string):
        return string
    else:
        raise NotADirectoryError(string)

# This program performs an analysis of the given
# source text file and builds a list of occurances
# of each character, character pair, character triple
# and word.
#
# This data is then used to create a dictionary lookup
#
# This program assumes the input is line oriented. Consequently
# newlines are not encoded

parser = argparse.ArgumentParser()
parser.add_argument(
    "source",
    help="file to build a dictionary for")
parser.add_argument(
    "-n",
    "--name",
    default="String",
    help="C Variable Name to use")
parser.add_argument(
    "-c",
    "--comment-char",
    dest="comment",
    default='#',
    help="ignore lines with the given character")

args = parser.parse_args()

lines = list()
with open(args.source) as f:
    for line in f:
        if args.comment and line.strip().startswith(args.comment):
            continue
        elif line:
            lines.append(line)

encoded = list()
for line in lines:
    bytestr = fbp.encode(line)
    if bytestr:
        encoded.append(bytestr)

def asBytes(sequence):
    return ", ".join(["0x%02x" % ord(ch) for ch in sequence])

data = dict(
    encoded=encoded)

loader = jinja2.FileSystemLoader(pathlib.Path(__file__).parent)
env = jinja2.Environment(loader=loader, trim_blocks=True)
env.globals["asBytes"] = asBytes
env.globals["hex"] = hex

outfile = os.path.join(args.directory, "Dictionary.c")
template = env.get_template("EncodeFile.c.jinja")
result = template.render(**data)
with open(outfile, "w+") as f:
    f.write(result)

outfile = os.path.join(args.directory, "Dictionary.h")
template = env.get_template("EncodeFile.h.jinja")
result = template.render(**data)
with open(outfile, "w+") as f:
    f.write(result)
