import os
import os.path
import sys

TAG = "=== File: "

def parse(content):
    result = {}
    current_file = None
    for line in content:
        if line.startswith(TAG):
            current_file = get_line(line.replace(TAG, ''))
        if line.startswith("O("):
            complexity = line[line.find("(")+1:line.find(")")]
            if not current_file:
                raise Exception("Invalid file format")
            result[current_file] = complexity
            current_file = None
    return result

def get_line(filename):
    content = filename.split("_")
    if not ".csv" in content[-1]:
        raise Exception("Invalid filename " + filename)
    return content[-1][:content[-1].find(".csv")]

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

    file = open(filename, 'r')
    content = file.readlines()
    file.close()

    equation = "(Line 8)*(Line 15)"
    result = parse(content)
    complexity = equation
    for key in result:
        complexity = complexity.replace("Line " + key, result[key])

    print "O(" + complexity + ")"

