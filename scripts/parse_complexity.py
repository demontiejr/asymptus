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

def parse_equations(equations):
    result = {}
    current_function = None
    for line in equations:
        if line.startswith("Function"):
            if current_function:
                raise Exception("Invalid file format")
            current_function = line.split(' ')[1].strip()
        else:
            equation = line.strip()
            if not current_function:
                raise Exception("Invalid file format")
            if equation:
                result[current_function] = equation
            current_function = None
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
    if len(sys.argv) != 3:
        print "Usage: python %s equations_filename filename" % sys.argv[0]
        exit()
    
    eq_filename = sys.argv[1]
    filename = sys.argv[2]

    file = open(eq_filename, 'r')
    equations = file.readlines()
    file.close()

    file = open(filename, 'r')
    content = file.readlines()
    file.close()

    equations = parse_equations(equations)
    result = parse(content)
    for function in equations:
        complexity = equations[function]
        for key in result:
            complexity = complexity.replace("Line " + key, result[key])

        print "Complexity of function '%s': O(%s)" % (function, complexity)

