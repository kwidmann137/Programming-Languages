import re
from p5team import nickNamesD

lossAliasL = ["LOSES TO",
              "LOST TO",
              "BEAT BY",
              "BEATEN BY",
              "DEFEATED BY",
              "SLAUGHTERED BY",
              "WHIPPED BY",
              "TOPPED BY",
              "UPSET BY",
              "EATEN BY",
              "DESTROYED BY"]
winAliasL = ["BEAT", "BEATS",
             "DEFEAT", "DEFEATS", "DEFEATED",
             "SLAUGHTER", "SLAUGHTERS", "SLAUGHTERED",
             "WHIP", "WHIPS", "WHIPPED",
             "TOP", "TOPS", "TOPPED",
             "UPSET", "UPSETS",
             "SHUT", "SHUTS", "SHUTS OUT", "EAT", "EATS",
             "DESTROY", "DESTROYS", "DESTROYED",
             "SOAR", "SOARS", "SOAR OVER", "SOARS OVER"]


def getTeams(line):
    team1 = ""
    index1 = -1
    team2 = ""
    index2 = -1

    # first check if team name matches
    for team, nicknames in nickNamesD.items():
        reString = r'\b' + re.escape(team.strip()) + r'\b'
        teamMatch = re.search(reString, line)
        if teamMatch is not None:
            if team1 == "" and index1 == -1:
                team1 = team.strip()
                index1 = teamMatch.start()
            else:
                team2 = team.strip()
                index2 = teamMatch.start()
        # then check if nickname matches
        else:
            for nickname in nicknames:
                reString = r'\b' + re.escape(nickname.strip()) + r'\b'
                nicknameMatch = re.search(reString, line)
                if nicknameMatch is not None:
                    if team1 == "" and index1 == -1:
                        team1 = team.strip()
                        index1 = nicknameMatch.start()
                    else:
                        team2 = team.strip()
                        index2 = nicknameMatch.start()

    # if both teams were not found return None
    # otherwise return according to order in sentence
    if index1 == -1 or index2 == -1:
        return None
    elif index1 < index2:
        return team1, team2
    else:
        return team2, team1


def determineIfFirstTeamWon(line):
    # if the splashy is a loss alias then the first team did not win
    for splashy in lossAliasL:
        if re.search(re.escape(splashy.strip()), line):
            return False

    # if the splashy is a win alias then the first team did win
    for splashy in winAliasL:
        if re.search(re.escape(splashy.strip()), line):
            return True

    # if splashy was not found return None
    return None


def getScore(line):
    scoreRE = re.search(r'(\d+)[^\d]*(\d+)', line)
    if scoreRE is not None:
        return int(scoreRE.group(1)), int(scoreRE.group(2))
    return None


def parseScoreLine(scoreLine):
    team1, team2 = getTeams(scoreLine)
    firstTeamWon = determineIfFirstTeamWon(scoreLine)
    score1, score2 = getScore(scoreLine)
    return team1, team2, firstTeamWon, score1, score2
