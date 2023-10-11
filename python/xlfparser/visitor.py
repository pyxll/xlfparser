from .types import Token, Type
from abc import ABCMeta, abstractmethod


class Visitor(metaclass=ABCMeta):

    def visit(self, token: Token):
        if token.type == Type.Unknown:
            self.visit_unknown(token)
        elif token.type == Type.Operand:
            self.visit_operand(token)
        elif token.type == Type.Function:
            self.visit_function(token)
        elif token.type == Type.Array:
            self.visit_array(token)
        elif token.type == Type.ArrayRow:
            self.visit_arrayrow(token)
        elif token.type == Type.Subexpression:
            self.visit_subexpression(token)
        elif token.type == Type.Argument:
            self.visit_argument(token)
        elif token.type == Type.OperatorPrefix:
            self.visit_operatorprefix(token)
        elif token.type == Type.OperatorInfix:
            self.visit_operatorinfix(token)
        elif token.type == Type.OperatorPostfix:
            self.visit_operatorpostfix(token)
        elif token.type == Type.Whitespace:
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


def visit(tokens: list[Token], visitor: Visitor):
    """Call the visitor for each token from a tokenized Excel formula."""
    for token in tokens:
        visitor.visit(token)
