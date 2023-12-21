#!/usr/bin/env python3
import os
import pathlib
import argparse
import jinja2

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
    "-t",
    "--no-trim",
    dest="trim",
    action="store_false",
    help="do not trim whitespace from input lines")
parser.add_argument(
    "-c",
    "--comment-char",
    dest="comment",
    default='#',
    help="ignore lines with the given character")
parser.add_argument(
    "-d",
    "--directory",
    default=".",
    type=isDir,
    help="Directory to write generated files to")
parser.add_argument(
    "-m",
    "--max",
    default=(256 - RESERVED_CODES),
    help=f"size of the resulting dictionary, must be <= {256 - RESERVED_CODES}."
        "smaller size means less space taken up by the"
        "compression dictionary but a worse compression ratio")
args = parser.parse_args()

# build a dictionary of 254 values. text is then
# encoded (line by line) as a stream of bytes.
#
# bytes are interpreted by the decompressor as:
#   0x00 = end of string
#   0x01 = literal (followed by len byte then literals)
#   0x02 = literal (1 character follows)
#   * = index into dictionary
#
# dictionary entries can be:
#   - 1 character long
#   - 2 characters long
#   - 3 characters long
# The dictionary is built starting w/ 1 char, then 2 chars, etc...
# this allows dropping the \0 terminator from known character
# size entries
#
# To determine the dictionary, we first insert all base glyphs.
#   for example this might be all lower (26) and uppercase letters (26)
#   and a few punctuations (4) and the space character (1) giving us
#   253-57 = 196 free slots in the dictionary.
# Then we create a mapping of occurrence count to lists of 2 characters,
# and 3 characters. The mapping is then sorted by
# occurrences and the remaining dictionary slots are filled with the
# most commonly occurring mappings.
#
# In the common case, common tuples are compressed 50%, common triples
# compressed 66%

lines = list()
with open(args.source) as f:
    for line in f:
        if args.comment and line.startswith(args.comment):
            continue
        elif args.trim:
            lines.append(line.strip())
        else:
            lines.append(line)

class Counter:
    def __init__(self, sequence, count = 0):
        self.seq = sequence
        self.num = count
    def __lt__(self, other):
        return (self.num, self.seq) < (other.num, other.seq)
    def __repr__(self):
        return f"('{self.seq}', {self.num})"

def inc(dst, seq):
    existing = dst.get(seq, Counter(seq, 0))
    existing.num += 1
    dst[seq] = existing

uniqueGlyphs = dict()
# determine how many unique characters we have.
# these form the basis of the dictionary.
# if the input text only uses uppercase letters and spaces
# we only need 26+1 slots in the dictionary.
# likewise if the text doesn't use 'z' for example, we can leave it out.
for line in lines:
    for char in line:
        inc(uniqueGlyphs, char)

uniqueTuples = dict()
# determine how many unique pairs of glyphs we have.
# for example, "th" is a common glyph pair
for line in lines:
    for i in range(0, len(line) - 1):
        sequence = line[i:i+2]
        inc(uniqueTuples, sequence)

uniqueTriples = dict()
# determine how many unique triples of glyphs we have.
# for example, "the" is a common glyph triple
for line in lines:
    for i in range(0, len(line) - 3):
        sequence = line[i:i+3]
        inc(uniqueTriples, sequence)

uniques = dict()
# now join all our unique tuples, pairs, and words then sort
# by total occurance count. We then take the top
# 254 entries - uniqueGlyphs to form our dictionary.
uniques.update(uniqueGlyphs)
uniques.update(uniqueTuples)
uniques.update(uniqueTriples)
uniques = list(uniques.values())
uniques.sort(reverse=True)

# # print all pairs and frequencies for debugging
# for e in uniques:
#     print(f"{e.seq}: {e.num}")

# TODO:
# Determine if some single characters could be encoded only as double characters
# Determine if some double characters could be encoded as only triple characters
#  If a characters count == the count of all double/triples containing it
#  then we can drop the single character in favor of the double/triple

# CONSIDER:
# If we move to a bit oriented protocol, could reserve 3/4 bit patterns
# for the most commonly occuring single/tuple/triple. this bites into
# the dictionary size a bit or requires adding a header bit. not sure
# if it is worth it

entries = uniques[:args.max]
entries.sort(key = lambda x: len(x.seq))
tupleStartIdx = 255
tripleStartIdx = 255
tupleStartOffset = 0
tripleStartOffset = 0
offset = RESERVED_CODES
for i in range(0, len(entries)):
    entry = entries[i]
    if tupleStartIdx == 255 and len(entry.seq) == 2:
        tupleStartIdx = i + RESERVED_CODES
        tupleStartOffset = offset
    elif tripleStartIdx == 255 and len(entry.seq) == 3:
        tripleStartIdx = i + RESERVED_CODES
        tripleStartOffset = offset
    offset += len(entry.seq)

def asBytes(sequence):
    return ", ".join(["0x%02x" % ord(ch) for ch in sequence])

data = dict(
    entries=entries,
    tupleStartIdx=tupleStartIdx,
    tripleStartIdx=tripleStartIdx,
    tupleStartOffset=tupleStartOffset,
    tripleStartOffset=tripleStartOffset,
    totalEntries=len(entries),
    totalBytes=offset,
    RESERVED_CODES=RESERVED_CODES)


rootdir = pathlib.Path(__file__).parent.parent
loader = jinja2.FileSystemLoader(rootdir / "res" / "templates")
env = jinja2.Environment(loader=loader, trim_blocks=True)
env.globals["asBytes"] = asBytes
env.globals["hex"] = hex

outfile = os.path.join(args.directory, "Dictionary.c")
template = env.get_template("Dictionary.c.jinja")
result = template.render(**data)
with open(outfile, "w+") as f:
    f.write(result)

outfile = os.path.join(args.directory, "Dictionary.h")
template = env.get_template("Dictionary.h.jinja")
result = template.render(**data)
with open(outfile, "w+") as f:
    f.write(result)
