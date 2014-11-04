import os
import os.path
import sys
from collections import defaultdict

TAG = "<LoopInstr>"

def parse(file_lines):
    result_map = defaultdict(list)
    for line in file_lines:
        if not line.startswith(TAG) or line.count(":") != 1:
            continue
        line = line.replace(TAG+" ", "")
        content = line.split(":")
        result_map[content[0].strip()].append(content[1].strip())
    return result_map

def write(filename, content_map):
    outfile = open(filename, 'w')
    for k in content_map:
        outfile.write("---" + k + "---\n")
        for value in result[k]:
            outfile.write(" " + value + "\n")

    outfile.close() 


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "Usage: python %s filename" % sys.argv[0]
        exit()
    
    filename = sys.argv[1]

    print "Reading", filename
    file = open(filename, 'r')
    content = file.readlines()
    file.close()

    result = parse(content)
    outfile = filename + ".out-instr"
    write(outfile, result)
