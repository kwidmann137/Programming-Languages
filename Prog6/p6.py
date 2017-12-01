import sys
import re
import operator
from Exceptions.FunctionError import FunctionError
from Exceptions.PrefixSyntax import PrefixSyntax

# DICTIONARY OF OPERATIONS ALLOWED
ops = {"+": operator.add,
       "-": operator.sub,
       "*": operator.mul,
       "/": operator.truediv,
       ">": operator.gt,
       "<": operator.lt,
       "and": operator.and_,
       "or": operator.or_}


def get_line():
    """
    Gets the next line from the input file
    :return: the next lien or None if EOF
    """
    current_line = inputFile.readline()
    if current_line == "":
        return None
    current_line = current_line.strip()
    return current_line


def parse_prefix(line):
    """
    Parses the given line into an array of each prefix operator/operand.
    Can throw an error if invalid prefix syntax given
    :param line: the line to parse
    :return: array of operators/operands in order from the line
    """
    valid_tokens = re.findall(r'[+-/*//()><]|or|and|\d+', line)
    all_tokens = re.findall(r'[+-/*//()><]|or|and|\w+|\d+', line)
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
                raise PrefixSyntax("Encountered operator({}) not in parentheses in function: {}".format(token, line))
        elif prev_token == '(':
            raise FunctionError("Invalid operation supplied: {} in function {}".format(token, line))
        else:
            raise PrefixSyntax("Excepted an integer, but found str: {} in function {}".format(token, line))

        prev_token = token

    if not has_operands:
        raise PrefixSyntax("Missing operands from statement: {}".format(line))
    elif closing_paren < opening_paren:
        raise PrefixSyntax("Missing closing parentheses: {}".format(line))
    elif opening_paren < closing_paren:
        raise PrefixSyntax("Missing opening parentheses: {}".format(line))

    return valid_tokens


def is_number(s):
    """
    Check if the given string is a valid number
    :param s: string to evaluate
    :return: true if s is a number, false otherwise
    """
    try:
        float(s)
        return True
    except ValueError:
        pass

    return False


def eval_operation(operation, operands):
    """
    evaluates a given operation on an array operands
    :param operation: the operation to perform
    :param operands: the operands to perform the operation on
    :return: the result of the operation on all operands
    """
    if len(operands) < 2:
        raise FunctionError("Operation with less than two operands. '{}' requires 2.".format(operation))
    result = operands[0]
    for index in range(1, len(operands), 1):
        result = ops[operation](result, operands[index])
    return result


def eval_prefix(tokens):
    """
    Evaluates an array of tokens in prefix syntax
    :param tokens: tokens to evaluate
    :return: the result from evaluating the prefix tokens
    """
    global currTokenPosition
    operands = []
    curr_token = tokens[currTokenPosition]

    if is_number(curr_token):
        currTokenPosition = currTokenPosition + 1
        return float(curr_token)

    # set the operation and advance token
    operation = tokens[currTokenPosition + 1]
    currTokenPosition = currTokenPosition + 2
    curr_token = tokens[currTokenPosition]

    # append all values after the operation until closing paren
    while curr_token != ')':
        operands.append(eval_prefix(tokens))
        curr_token = tokens[currTokenPosition]

    # advance position past closing paren and return eval
    currTokenPosition = currTokenPosition + 1
    return eval_operation(operation, operands)


def prefix_reader(line):
    """
    Parses a line with prefix syntax and evaluates it if valid
    :param line: the string to parse and evaluate
    """
    global currTokenPosition
    print("> {}".format(line))
    try:
        tokens = parse_prefix(line)
        result = eval_prefix(tokens)
        print(result)
    except FunctionError as e:
        print(e.message)
    except PrefixSyntax as e:
        print(e.message)

    currTokenPosition = 0


# ******************** MAIN LOOP ************************

# Get the file to read
inputFileName = sys.argv[1]
# open the file
inputFile = open(inputFileName, "r")
# get the first line
currentLine = get_line()
# init token position
currTokenPosition = 0

# read all lines from file and evaluate
while currentLine is not None:
    prefix_reader(currentLine)
    currentLine = get_line()
