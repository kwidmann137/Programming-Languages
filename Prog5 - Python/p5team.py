nickNamesD = {}
teamScoreD = {}


def initTeamRecord():
    for team in nickNamesD.keys():
        teamScoreD[team] = [0, 0]


def setTeamNicknames(line):
    teamNames = line.split(' ', 1)
    actualName = teamNames[0]
    nickNamesL = teamNames[1].split(',')
    map(str.strip, nickNamesL)
    nickNamesD[actualName] = nickNamesL


def addLossToTeamRecord(team):
    currentRecord = teamScoreD[team]
    currentRecord[1] = currentRecord[1] + 1


def addWinToTeamRecord(team):
    currentRecord = teamScoreD[team]
    currentRecord[0] = currentRecord[0] + 1
