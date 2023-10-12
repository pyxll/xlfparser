from dataclasses import dataclass
from abc import ABCMeta, abstractmethod
from enum import Enum
from .options import Options, OptionsKwargs


@dataclass
class Token:

    class Type(Enum):
        Unknown = 0
        Operand = 1
        Function = 2
        Array = 3
        ArrayRow = 4
        Subexpression = 5
        Argument = 6
        OperatorPrefix = 7
        OperatorInfix = 8
        OperatorPostfix = 9
        Whitespace = 10


    class SubType(Enum):
        _None = 0
        Start = 1
        Stop = 2
        Text = 3
        Number = 4
        Logical = 5
        Error = 6
        Range = 7
        Math = 8
        Concatenation = 9
        Intersection = 10
        Union = 11

    value: str
    type: Type
    sub_type: SubType


class TokenVisitor(metaclass=ABCMeta):

    def visit(self, token: Token):
        if token.type == Token.Type.Unknown:
            self.visit_unknown(token)
        elif token.type == Token.Type.Operand:
            self.visit_operand(token)
        elif token.type == Token.Type.Function:
            self.visit_function(token)
        elif token.type == Token.Type.Array:
            self.visit_array(token)
        elif token.type == Token.Type.ArrayRow:
            self.visit_arrayrow(token)
        elif token.type == Token.Type.Subexpression:
            self.visit_subexpression(token)
        elif token.type == Token.Type.Argument:
            self.visit_argument(token)
        elif token.type == Token.Type.OperatorPrefix:
            self.visit_operatorprefix(token)
        elif token.type == Token.Type.OperatorInfix:
            self.visit_operatorinfix(token)
        elif token.type == Token.Type.OperatorPostfix:
            self.visit_operatorpostfix(token)
        elif token.type == Token.Type.Whitespace:
            self.visit_whitespace(token)
        else:
            raise ValueError(f"Unexpected token {token=}")

    @abstractmethod
    def visit_unknown(self, token: Token):
        pass

    @abstractmethod
    def visit_operand(self, token: Token):
        pass

    @abstractmethod
    def visit_function(self, token: Token):
        pass

    @abstractmethod
    def visit_array(self, token: Token):
        pass

    @abstractmethod
    def visit_arrayrow(self, token: Token):
        pass

    @abstractmethod
    def visit_subexpression(self, token: Token):
        pass

    @abstractmethod
    def visit_argument(self, token: Token):
        pass

    @abstractmethod
    def visit_operatorprefix(self, token: Token):
        pass

    @abstractmethod
    def visit_operatorinfix(self, token: Token):
        pass

    @abstractmethod
    def visit_operatorpostfix(self, token: Token):
        pass

    @abstractmethod
    def visit_whitespace(self, token: Token):
        pass


class StringifyVisitor(TokenVisitor):

    def __init__(self, options: Options):
        self.options = options
        self.values = []

    @property
    def value(self):
        return "".join(self.values)

    def visit_unknown(self, token: Token):
        self.values.append(token.value)

    def visit_operand(self, token: Token):
        self.values.append(token.value)

    def visit_function(self, token: Token):
        if token.sub_type == Token.SubType.Start:
            self.values.append(token.value)
            self.values.append("(")
        elif token.sub_type == Token.SubType.Stop:
            self.values.append(")")

    def visit_array(self, token: Token):
        if token.sub_type == Token.SubType.Start:
            self.values.append(self.options.left_brace)
        elif token.sub_type == Token.SubType.Stop:
            if self.values[-1] == self.options.row_separator:
                self.values.pop()
            self.values.append(self.options.right_brace)

    def visit_arrayrow(self, token: Token):
        if token.sub_type == Token.SubType.Stop:
            self.values.append(self.options.row_separator)

    def visit_subexpression(self, token: Token):
        self.values.append(token.value)

    def visit_argument(self, token: Token):
        self.values.append(self.options.list_separator)

    def visit_operatorprefix(self, token: Token):
        self.values.append(token.value)

    def visit_operatorinfix(self, token: Token):
        if token.sub_type == Token.SubType.Union:
            self.values.append(self.options.list_separator)
        else:
            self.values.append(token.value)

    def visit_operatorpostfix(self, token: Token):
        self.values.append(token.value)

    def visit_whitespace(self, token: Token):
        pass


def stringify(tokens: list[Token], prefix="=", **kwargs: OptionsKwargs) -> str:
    """Convert a list of tokens returned from tokenize back into a string."""
    options = Options.from_kwargs(**kwargs)
    visitor = StringifyVisitor(options)
    for token in tokens:
        visitor.visit(token)
    return prefix + visitor.value
