import os
import os.path
import sys
from collections import defaultdict

TAG = "<LoopInstr>"

class Value:

    def __init__(self, variable, value, isIV=False):
        self.variable = variable
        self.value = value
        self.isIV = isIV

    def __str__(self):
        return "%s = %s" % (self.variable, self.value)

    def __repr__(self):
        return self.__str__()

def parse_to_values(file_lines):
    result_map = defaultdict(list)
    for line in file_lines:
        if not line.startswith(TAG) or line.count(":") != 1:
            continue
        line = line.replace(TAG+" ", "")

        if line.startswith("<IV>"):
            isIV = True
            line = line.replace("<IV> ", "")
        else:
            isIV = False

        content = line.split(":")
        var = content[1].split("=")
        if len(var) < 2:
            continue
        result_map[content[0].strip()].append(Value(var[0].strip(), var[1].strip(), isIV))
    return result_map

def parse(file_lines):
    result_map = defaultdict(list)
    for line in file_lines:
        if not line.startswith(TAG) or line.count(":") != 1:
            continue
        line = line.replace(TAG+" ", "").replace("<IV> ", "")
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
