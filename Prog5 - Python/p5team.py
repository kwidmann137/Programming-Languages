"""
Dictionary that holds a teams nicknames
Key: Official Name ex) UTSA
Value: A list of nicknames for tje team
"""
nickNamesD = {}

"""
Dictionary to hold a teams record
Key: Teams official name
Value: a list where index 0 is wins and index 1 is loses
"""
teamRecordD = {}


def init_team_records():
    """
    Initializes all team records to 0 wins 0 loses
    """
    for team in nickNamesD.keys():
        teamRecordD[team] = [0, 0]


def set_team_nicknames(line):
    """
    Sets a teams nicknames from a line with the following format
    officialName nickname1, nickname2...
    :param line: The line to parse for nicknames
    """
    team_names = line.split(' ', 1)
    actual_name = team_names[0]
    nick_names_list = team_names[1].split(',')
    map(str.strip, nick_names_list)
    nickNamesD[actual_name] = nick_names_list


def add_loss_to_team_record(team):
    """
    Adds a loss to the given teams record
    :param team: Team to add a loss to
    """
    current_record = teamRecordD[team]
    current_record[1] = current_record[1] + 1


def add_win_to_team_record(team):
    """
    Adds a win to the given teams record
    :param team: Team to add a win to
    """
    current_record = teamRecordD[team]
    current_record[0] = current_record[0] + 1


def update_team_records_from_score(team1, team2, first_team_won, score1, score2):
    """
    Updates the records for both teams given the first team boolean and scores
    :param team1: first team in the score line
    :param team2: second team in the score line
    :param first_team_won: boolean if first team won or not, might be None if invalid verb
    :param score1: first score from score line
    :param score2: second score from score line
    """
    if first_team_won is None:
        if score1 > score2:
            add_win_to_team_record(team1)
            add_loss_to_team_record(team2)
        else:
            add_win_to_team_record(team2)
            add_loss_to_team_record(team1)
    elif first_team_won:
        add_win_to_team_record(team1)
        add_loss_to_team_record(team2)
    elif not first_team_won:
        add_win_to_team_record(team2)
        add_loss_to_team_record(team1)


def print_team_records():
    """
    Prints the records of all teams in the record dictionary
    """
    print("\t\t%-8s%-5s%-5s" % ("TEAM", "WINS", "LOSES"))
    for team, record in teamRecordD.items():
        print("\t\t%-8s%-5s%-5s" % (team, record[0], record[1]))
