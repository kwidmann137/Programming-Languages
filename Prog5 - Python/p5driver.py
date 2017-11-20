#!/usr/bin/python3.5
import sys
from p5week import *
from p5team import *

inputFileName = sys.argv[1]
inputFile = open(inputFileName, "r")
currentLine = ""

weekRE = re.compile(r'(WEEK) (\d+)')


def getLine():
    global currentLine
    currentLine = inputFile.readline()
    if currentLine == "":
        return None
    return currentLine.strip()


def initTeamNicknames():
    global currentLine
    # flush current line with first team nickname line
    currentLine = getLine()
    while currentLine is not None and currentLine != "ENDTEAMS":
        setTeamNicknames(currentLine)
        currentLine = getLine()


def getWeekScores():
    global currentLine
    # get the first score line after the begin week command
    currentLine = getLine()
    while currentLine is not None and currentLine != "ENDWEEK":
        print(">>>", currentLine)
        team1, team2, firstTeamWon, score1, score2 = parseScoreLine(currentLine)
        printOfficialResult(team1, team2, firstTeamWon, score1, score2)
        currentLine = getLine()


def printScores():
    print("\t\t%-8s%-5s%-5s" % ("TEAM", "WINS", "LOSES"))
    for team, record in teamScoreD.items():
        print("\t\t%-8s%-5s%-5s" % (team, record[0], record[1]))


def printOfficialResult(team1, team2, firstTeamWon, score1, score2):
    if score1 > score2:
        highScore = score1
        lowScore = score2
    else:
        highScore = score2
        lowScore = score1

    if firstTeamWon is None:
        if score1 > score2:
            addWinToTeamRecord(team1)
            addLossToTeamRecord(team2)
            print("\t\tUnknown verb, by score, assume team 1 won")
            print("\t\t%s %s %d %d" % (team1, team2, score1, score2))
        else:
            addWinToTeamRecord(team2)
            addLossToTeamRecord(team1)
            print("\t\tUnknown verb, by score, assume team 2 won")
            print("\t\t%s %s %d %d" % (team2, team1, score2, score1))
    elif firstTeamWon:
        addWinToTeamRecord(team1)
        addLossToTeamRecord(team2)
        print("\t\t%s %s %d %d" % (team1, team2, highScore, lowScore))
    elif not firstTeamWon:
        addWinToTeamRecord(team2)
        addLossToTeamRecord(team1)
        print("\t\t%s %s %d %d" % (team2, team1, highScore, lowScore))
    else:
        print("Error parsing score line")


while currentLine is not None:
    if currentLine == "TEAMS":
        initTeamNicknames()
        initTeamRecord()
        print("Initial")
        printScores()
    elif weekRE.match(currentLine):
        print(">>>", currentLine)
        getWeekScores()
        print(">>> ENDWEEK")
        printScores()

    currentLine = getLine()
