import os
import os.path
import sys
from collections import defaultdict

TAG = "=== File: "

class Element:
    
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return str(self.value)

    def var(self):
        return self.value.split("^")[0]

    def exp(self):
        split = self.value.split("^")
        return int(split[1]) if len(split) == 2 else 1

    def simplify(self):
        return self

    def __repr__(self):
        return self.__str__()

class Expression:

    def __init__(self):
        self.values = []
        self.operator = ''

    def add(self, value):
        self.values.append(value)

    def simplify(self):
        for i in xrange(len(self.values)):
            e = self.values[i].simplify()
            if e:
                self.values[i] = e
            else:
                self.values.pop(i)

        return self

    def __str__(self):
        result = ""
        for v in self.values:
            result += str(v) + (' %s ' % self.operator)
        result = result[:-3]
        return "(%s)" % result if len(self.values) > 1 else result

    def __repr__(self):
        return self.__str__()

class Sum(Expression):
    def __init__(self):
        Expression.__init__(self)
        self.operator = '+'

    def simplify(self):
        Expression.simplify(self)
        
        if len(self.values) == 0:
            return None
        else:
            new_values = []
            greatest = defaultdict(int)
            for v in self.values:
                if isinstance(v, Element):
                    var = v.var()
                    exp = v.exp()
                    if greatest[var] < exp:
                        greatest[var] = exp
                else:
                    new_values.append(v)

            for k,v in greatest.items():
                new_values.append(Element(k + ('^' + str(v) if v > 1 else '')))
                
            self.values = new_values

        if len(self.values) == 1:
            return self.values[0]
        return self

class Mul(Expression):
    def __init__(self):
        Expression.__init__(self)
        self.operator = '*'

    def simplify(self):
        def multiply(exps_map, element):
            var = element.var()
            exp = element.exp()
            exps_map[var] += exp

        Expression.simplify(self)

        if len(self.values) == 0:
            return None
        else:
            new_values = []
            exps = defaultdict(int)
            distributive = Sum()
            for v in self.values:
                if isinstance(v, Element):
                    multiply(exps, v)
                elif isinstance(v, Sum):
                    if not distributive.values:
                        for i in v.values:
                            mul = Mul()
                            mul.add(i)
                            distributive.add(mul)
                    else:
                        tmp = []
                        for i in distributive.values:
                            for j in v.values:
                                mul = Mul()
                                mul.values = i.values[0:]
                                mul.add(j)
                                tmp.append(mul)
                        distributive.values = tmp
                else:
                    for i in v.values:
                        multiply(exps, i)
            
            for k,v in exps.items():
                new_values.append(Element(k + ('^' + str(v) if v > 1 else '')))

            self.values = new_values

        if distributive.values:
            if self.values:
                tmp = []
                for i in distributive.values:
                    for j in self.values:
                        mul = Mul()
                        mul.values = i.values[0:]
                        mul.add(j)
                        tmp.append(mul)
                distributive.values = tmp
            return distributive.simplify()

        if len(self.values) == 1:
            return self.values[0] 
        return self


def parse(content):
    result = defaultdict(dict)
    current_file = None
    for line in content:
        if line.startswith(TAG):
            line = line.replace(TAG, '')
            current_file = get_line(line)
            current_function = get_function(line)
        elif line.startswith("O("):
            if not current_file:
                raise Exception("Invalid file format")
            complexity = line[line.find("(")+1:line.find(")")]
            result[current_function][current_file] = {"comp": complexity}
        elif line.startswith("Polynomial"):
            if not current_file or current_file not in result[current_function]:
                raise Exception("Invalid file format")
            polynomial = line.split("=")[1].strip()
            result[current_function][current_file]["poly"] = polynomial
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
            #if not current_function:
            #    raise Exception("Invalid file format")
            if equation:
                result[current_function] = equation
            current_function = None
    return result

def get_line(filename):
    content = filename.split("_")
    if not ".csv" in content[-1]:
        raise Exception("Invalid filename " + filename)
    return content[-1][:content[-1].find(".csv")]

def get_function(filename):
    return filename.split("#")[1]

def simplify(complexity):
    if '@' in complexity:
        return complexity
    complexity = complexity.replace("(", "( ").replace(")", " )")
    eq = Sum()
    stack = []
    stack.append(eq)
    stack.append(Mul())
    for element in complexity.split(' '):
        if element.strip() == "(":
            stack.append(Sum())
            stack.append(Mul())
        elif element.strip() == ")":
            if len(stack) < 2:
                raise Exception("Invalid equation")
            mul_exp = stack.pop()
            sum_exp = stack.pop()
            sum_exp.add(mul_exp)
            stack[-1].add(sum_exp)
        elif element.strip() == "+":
            stack[-2].add(stack.pop())
            stack.append(Mul())
        elif element.strip() == "*":
            continue
        else:
            stack[-1].add(Element(element.strip()))

    while len(stack) > 1:
        stack[-2].add(stack.pop())

    return eq.simplify()
    
def write(filename, content_map):
    outfile = open(filename, 'w')
    for k in content_map:
        outfile.write("---" + k + "---\n")
        for value in result[k]:
            outfile.write(" " + value + "\n")

    outfile.close() 


if __name__ == "__main__":
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print "Usage: python %s equations_filename filename [-l]" % sys.argv[0]
        exit()
    
    loop_grain = False
    if len(sys.argv) == 4:
        loop_grain = True
    
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
        if function not in result:
            continue
        
        print "Function '%s':" % function
        complexity = equations[function]
        
        for key in sorted(result[function]):
            complexity = complexity.replace("Line" + key, result[function][key]['comp'])
            if loop_grain:
                polynomial = result[function][key]['poly']
                print "\tLoop at line %s: %s" % (key, polynomial)

        complexity = simplify(complexity)

        print "\n\tComplexity: O(%s)\n" % complexity

