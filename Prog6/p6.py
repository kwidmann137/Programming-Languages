import sys
import re
import operator

ops = { "+": operator.add,
        "-": operator.sub,
        "*": operator.mul,
        "/": operator.truediv,
        "and": operator.and_,
        "or": operator.or_}

inputFileName = sys.argv[1]

inputFile = open(inputFileName, "r")


def get_line():
    current_line = inputFile.readline()
    if current_line == "":
        return None
    current_line = current_line.strip()
    return current_line


def parse_prefix(line):
    return re.findall(r'[+-/*//()]|\d+', line)


def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        pass

    return False


def prefix_eval(tokens):
    global currTokenPosition

    operator = None
    operands = []
    result = None

    while currTokenPosition < len(tokens):
        print("CURR TOKEN {}".format(tokens[currTokenPosition]))
        if tokens[currTokenPosition] == '(':
            currTokenPosition = currTokenPosition + 1
            if operator is not None:
                print("RECURSING")
                print("OEPRATOR: {}".format(operator))
                operands.append(prefix_eval(tokens))
            else:
                operator = tokens[currTokenPosition]
                currTokenPosition = currTokenPosition + 1
        elif is_number(tokens[currTokenPosition]):
            operands.append(float(tokens[currTokenPosition]))
            currTokenPosition = currTokenPosition + 1
        elif tokens[currTokenPosition] == ')':
            currTokenPosition = currTokenPosition + 1
            result = operands[0]
            # print("OPERATOR: {}".format(operator))
            # print("OPERANDS: {}".format(operands))
            for index in range(1, len(operands), 1):
                # print("Index: {}".format(index))
                # print("Operator: {}".format(operator))
                # print("FUNC: {}".format(ops[operator]))
                # print("ARG: {}".format(operands[index]))
                # print("EVAL - curr result = {}".format(result))
                result = ops[operator](result, operands[index])

    print("FINAL RESULT: {}".format(result))
    return result


currentLine = get_line()


currTokenPosition = 0


while currentLine is not None:
    line_tokens = parse_prefix(currentLine)
    print("CURR LINE {}".format(currentLine))
    prefix_eval(line_tokens)
    currTokenPosition = 0
    currentLine = get_line()
