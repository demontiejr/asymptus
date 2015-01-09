import os
import os.path
import sys
from collections import defaultdict

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

def simplify(complexity):
    complexity = complexity.replace("(", "( ").replace(")", " )")
    stack = []
    eq = []
    stack.append(eq)
    for element in complexity.split(' '):
        if element.strip() == "(":
            new_l = []
            stack.append(new_l)
        elif element.strip() == ")":
            if len(stack) < 2:
                raise Exception("Invalid equation")
            new_l = stack.pop()
            stack[-1].append(new_l)
        else:
            stack[-1].append(element.strip())

    return ' '.join(resolve(eq))
    
def resolve(eq):
    if not eq:
        return []
    nested = get_nested_lists(eq)
    for i,l in nested:
        values = resolve(l)
        eq[i:i+1] = values

    while '*' in eq:
        modify = []
        for i in xrange(len(eq)):
            if eq[i] == '*' and var(eq[i-1]) == var(eq[i+1]):
                modify.append(i)

        for i in modify:
            eq[i-1:i+2] = [var(eq[i-1]) + '^' + str(exp(eq[i-1]) + exp(eq[i+1]))]

    greatest = defaultdict(int)
    for e in eq:
        if e in ('+', '*', '@', '(', ')'):
            continue
        v = var(e)
        degree = exp(e)
        if degree > greatest[v]:
            greatest[v] = degree

    eq = []

    for k,v in greatest.items():
        eq.append(k + ('^' + str(v) if v > 1 else ''))
        eq.append('+')
    eq.pop()
    
    return eq

def var(value):
    return value.split("^")[0]

def exp(value):
    split = value.split("^")
    return int(split[1]) if len(split) == 2 else 1

def get_nested_lists(outer_list):
    return [(i, outer_list[i]) for i in xrange(len(outer_list)) if isinstance(outer_list[i], list)]

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
            complexity = complexity.replace("Line" + key, result[key])

        complexity = simplify(complexity)

        print "Complexity of function '%s': O(%s)" % (function, complexity)

