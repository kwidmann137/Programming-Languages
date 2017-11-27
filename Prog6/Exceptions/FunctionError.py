class FunctionError(Exception):
    """
    An error when an exception is encountered while evaluating a function
    """
    def __init__(self, message):
        self.message = message
