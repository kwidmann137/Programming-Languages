#!/usr/bin/python3.5
import sys
from p5week import *
from p5team import *

"""
Name of the input file supplied via command line args
"""
inputFileName = sys.argv[1]

"""
The file object use to process input
"""
inputFile = open(inputFileName, "r")

"""
Global string used to represent the current line
"""
current_line = ""

"""
Regular expression to check for a week command
"""
weekRE = re.compile(r'(WEEK) (\d+)')


def get_line():
    """
    Gets the next line from the input file
    :return: The next line of the input file as a string
    """
    global current_line
    current_line = inputFile.readline()
    if current_line == "":
        return None
    return current_line.strip()


def init_team_nicknames():
    """
    Initializes team nicknames given after the TEAMS command
    """
    global current_line
    # flush current line with first team nickname line
    current_line = get_line()
    while current_line is not None and current_line != "ENDTEAMS":
        set_team_nicknames(current_line)
        current_line = get_line()


def process_week_scores():
    """
    Processes all scores for a WEEK command
    """
    global current_line
    # get the first score line after the begin week command
    current_line = get_line()
    while current_line is not None and current_line != "ENDWEEK":
        print(">>> {}".format(current_line))
        team1, team2, first_team_won, score1, score2 = parse_score_line(current_line)
        update_team_records_from_score(team1, team2, first_team_won, score1, score2)
        print_official_score(team1, team2, first_team_won, score1, score2)
        current_line = get_line()


def print_official_score(team1, team2, first_team_won, score1, score2):
    """
    Prints an official score from a splashy score line
    :param team1: first team from score line
    :param team2: second team form score line
    :param first_team_won: boolean if first team won, might be None if invalid splashy phrase
    :param score1: first score from score line
    :param score2: second score form score line
    """
    if score1 > score2:
        high_score = score1
        low_score = score2
    else:
        high_score = score2
        low_score = score1

    if first_team_won is None:
        if score1 > score2:
            print("\t\tUnknown verb, by score, assume team 1 won")
            print("\t\t{} {} {:d} {:d}".format(team1, team2, score1, score2))
        else:
            print("\t\tUnknown verb, by score, assume team 2 won")
            print("\t\t{} {} {:d} {:d}".format(team2, team1, score2, score1))
    elif first_team_won:
        print("\t\t{} {} {:d} {:d}".format(team1, team2, high_score, low_score))
    elif not first_team_won:
        print("\t\t{} {} {:d} {:d}".format(team2, team1, high_score, low_score))
    else:
        print("Error parsing score line")


# MAIN LOOP
while current_line is not None:
    if current_line == "TEAMS":
        init_team_nicknames()
        init_team_records()
        print("Initial")
        print_team_records()
    elif weekRE.match(current_line):
        print(">>> {}".format(current_line))
        process_week_scores()
        print(">>> ENDWEEK")
        print_team_records()

    current_line = get_line()
