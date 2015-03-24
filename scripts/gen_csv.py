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
        isIV = False
        build_name = True
        for value in content_map[k]:
            if "loopCounter" in value.variable:
                if 1 <= count <= 2:
                    current_line += "," + value.value + "\n"
                    if not current_line in lines:
                        lines.append(current_line)
                    current_line = ""
                    count = 0
                    build_name = False
                else:
                    lines = []
                    break
            else:
                count += 1
                if current_line:
                    current_line = str(abs(to_number(current_line) - to_number(value.value)))
                else:
                    current_line = value.value

                if build_name:
                    if not var_name:
                        var_name, isIV = value.variable, value.isIV
                    else:
                        if value.isIV and not isIV:
                            pass
                        elif isIV and not value.isIV:
                            var_name = value.variable
                        else:
                            var_name = var_name + "|" + value.variable

        if not lines:
            continue

        if k.startswith('.'):
            k = k[1:]

        write_to_file(k + ".csv", var_name, lines)

def to_number(value):
    try:
        f = float(value)
        i = int(f)
    except ValueError:
        raise Exception("Invalid file format: input value '" + str(value) + "' is not a number")
    else:
        return i if i == f else f

def readfile(filepath): 
    with open(filepath, 'r') as f: 
        for line in f:
            yield line

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "Usage: python %s filename" % sys.argv[0]
        exit()
    
    filename = sys.argv[1]

    print "Reading", filename
    #file = open(filename, 'r')
    #content = file.readlines()
    #file.close()
    content = readfile(filename)

    result = parse_to_values(content)
    write(result)
