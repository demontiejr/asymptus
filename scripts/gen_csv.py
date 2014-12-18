import os
import os.path
import sys
from collections import defaultdict
from parse_file import *

def write_to_file(file_name, var_name, lines):
    outfile = open(file_name, 'w')
    outfile.write(var_name + '\n')
    outfile.write(str(len(lines)) + '\n')
    outfile.writelines(lines)
    outfile.close()

def write(content_map):
    for k in content_map:
        lines = []
        count = 0
        current_line = ""
        var_name = ""
        for value in content_map[k]:
            value = value.split("=")
            if len(value) < 2:
                continue
            if "loopCounter" in value[0]:
                if count == 1:
                    current_line += "," + value[1].strip() + "\n"
                    if not current_line in lines:
                        lines.append(current_line)
                    current_line = ""
                    count = 0
                else:
                    lines = []
                    break
            else:
                count += 1
                current_line = value[1].strip()
                if not var_name:
                    var_name = value[0].strip()

        if not lines:
            continue

        if k.startswith('.'):
            k = k[1:len(k)]

        write_to_file(k + ".csv", var_name, lines)

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
    write(result)
