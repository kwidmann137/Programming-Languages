class PrefixSyntax(Exception):
    """
    An error when invalid prefix syntax is encountered
    """
    def __init__(self, message):
        self.message = message
