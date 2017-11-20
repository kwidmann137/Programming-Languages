import re
from p5team import nickNamesD

"""
List that contains all splashy phrases
indicating the first team listed lost
"""
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

"""
List that contains all splashy phrases
indicating the first team won
"""
winAliasL = ["BEAT", "BEATS",
             "DEFEAT", "DEFEATS", "DEFEATED",
             "SLAUGHTER", "SLAUGHTERS", "SLAUGHTERED",
             "WHIP", "WHIPS", "WHIPPED",
             "TOP", "TOPS", "TOPPED",
             "UPSET", "UPSETS",
             "SHUT", "SHUTS", "SHUTS OUT", "EAT", "EATS",
             "DESTROY", "DESTROYS", "DESTROYED",
             "SOAR", "SOARS", "SOAR OVER", "SOARS OVER"]


def get_teams_from_score_line(line):
    """
    Gets the first and second team listed in the score line
    :param line: the score line to parse
    :return: team1 - first team listed, team2 - second team listed
    """
    team1 = ""
    index1 = -1
    team2 = ""
    index2 = -1

    # first check if team name matches
    for team, nicknames in nickNamesD.items():
        re_string = r'\b' + re.escape(team.strip()) + r'\b'
        team_match = re.search(re_string, line)
        if team_match is not None:
            if team1 == "" and index1 == -1:
                team1 = team.strip()
                index1 = team_match.start()
            else:
                team2 = team.strip()
                index2 = team_match.start()
        # then check if nickname matches
        else:
            for nickname in nicknames:
                re_string = r'\b' + re.escape(nickname.strip()) + r'\b'
                nickname_match = re.search(re_string, line)
                if nickname_match is not None:
                    if team1 == "" and index1 == -1:
                        team1 = team.strip()
                        index1 = nickname_match.start()
                    else:
                        team2 = team.strip()
                        index2 = nickname_match.start()

    # if both teams were not found return None
    # otherwise return according to order in sentence
    if index1 == -1 or index2 == -1:
        return None
    elif index1 < index2:
        return team1, team2
    else:
        return team2, team1


def determine_if_first_team_won(line):
    """
    Determines if the first team won based off the splashy phrase
    :param line: the score line to parse
    :return: true if first team won, false if they lost, None if invalid splashy phrase
    """
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


def get_score_from_score_line(line):
    """
    Gets the scores from a score line
    :param line: score line to parse
    :return: score1 - first score, score2 - second score
    """
    score_reg_ex = re.search(r'(\d+)[^\d]*(\d+)', line)
    if score_reg_ex is not None:
        return int(score_reg_ex.group(1)), int(score_reg_ex.group(2))
    return None


def parse_score_line(line):
    """
    Parses score line to get teams, scores and if first team won
    :param line: score line to parse
    :return: team1 - first team listed,
        team2 - second team listed
        first_team_won - boolean if first team won, might be None
        score1 - first score
        score2 - second score
    """
    team1, team2 = get_teams_from_score_line(line)
    first_team_won = determine_if_first_team_won(line)
    score1, score2 = get_score_from_score_line(line)
    return team1, team2, first_team_won, score1, score2
