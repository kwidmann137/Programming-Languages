import sys
import re
import operator
from Exceptions.FunctionError import FunctionError
from Exceptions.PrefixSyntax import  PrefixSyntax

ops = { "+": operator.add,
        "-": operator.sub,
        "*": operator.mul,
        "/": operator.truediv,
        ">": operator.gt,
        "<": operator.lt,
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
    valid_tokens = re.findall(r'[+-/*//()><]|or|and|\d+', line)
    all_tokens = re.findall(r'[+-/*//()><]|or|and|\w+|\d+', line)
    has_operators = False
    has_operands = False
    opening_paren = 0
    closing_paren = 0
    prev_token = None
    for token in all_tokens:
        if token == "(":
            opening_paren = opening_paren + 1
        elif token == ")":
            closing_paren = closing_paren + 1
        elif is_number(token):
            has_operands = True
        elif token in ops:
            if prev_token is not "(":
                raise PrefixSyntax("Encountered operator({}) not in parentheses: {}".format(token, line))
            has_operators = True
        elif prev_token == '(':
            raise FunctionError("Invalid operation supplied: {}".format(token))
        else:
            raise PrefixSyntax("Excepted an integer, but found str: {}".format(token))

        prev_token = token

    if not has_operands or not has_operators:
        raise PrefixSyntax("Missing operator or operands form statement: {}".format(line))
    elif closing_paren < opening_paren:
        raise PrefixSyntax("Missing closing parentheses: {}".format(line))
    elif opening_paren < closing_paren:
        raise PrefixSyntax("Missing opening parentheses: {}".format(line))
    elif opening_paren == 0 or closing_paren == 0:
        raise PrefixSyntax("Missing parentheses: {}".format(line))

    return valid_tokens


def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        pass

    return False


def eval_operation(operation, operands):
    result = operands[0]
    if len(operands) < 2:
        raise FunctionError("Operation with less than two operands. '{}' requires 2.".format(operation))
    for index in range(1, len(operands), 1):
        result = ops[operation](result, operands[index])
    return result


def eval_prefix(tokens):
    global currTokenPosition
    operation = None
    operands = []

    while currTokenPosition < len(tokens):
        if tokens[currTokenPosition] == '(':
            if operation is not None:
                operands.append(eval_prefix(tokens))
            else:
                operation = tokens[currTokenPosition + 1]
                currTokenPosition = currTokenPosition + 2
        elif is_number(tokens[currTokenPosition]):
            operands.append(float(tokens[currTokenPosition]))
            currTokenPosition = currTokenPosition + 1
        elif tokens[currTokenPosition] == ')':
            currTokenPosition = currTokenPosition + 1
            return eval_operation(operation, operands)

    return None


def prefix_reader(line):
    global currTokenPosition
    print("> {}".format(line))
    try:
        tokens = parse_prefix(line)
        result = eval_prefix(tokens)
        print(result)
        currTokenPosition = 0
    except FunctionError as e:
        print(e.message)
    except PrefixSyntax as e:
        print(e.message)


currentLine = get_line()


currTokenPosition = 0


while currentLine is not None:
    prefix_reader(currentLine)
    currentLine = get_line()
