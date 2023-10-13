from abc import ABCMeta, abstractmethod
from .token import Token, TokenVisitor
from .options import OptionsKwargs, Options
from ._xlfparser import tokenize


class Node:
    """Node in an AST representation of a tokenized Excel formula."""

    token: Token
    children: list["Node"]

    def __init__(self, token=None, children=[]):
        self.token = token
        self.children = list(children)

    def to_string(self, prefix="=", separator="", **kwargs: OptionsKwargs) -> str:
        options = Options.from_kwargs(**kwargs)
        visitor = NodeToStringVisitor(prefix, separator, options)
        visitor.visit(self)
        return visitor.value

    def get_tokens(self, **kwargs: OptionsKwargs) -> list[Token]:
        options = Options.from_kwargs(**kwargs)
        visitor = NodeToTokensVisitor(options)
        visitor.visit(self)
        return visitor.tokens

    @property
    def tokens(self) -> list[Token]:
        return self.get_tokens()

    def __str__(self):
        return self.to_string()


class NodeVisitor(metaclass=ABCMeta):

    def visit(self, node: Node):
        if node.token is None:
            self.visit_children(node)
        elif node.token.type == Token.Type.Operand:
            self.visit_operand(node)
        elif node.token.type == Token.Type.Function:
            self.visit_function(node)
        elif node.token.type == Token.Type.Array:
            self.visit_array(node)
        elif node.token.type == Token.Type.ArrayRow:
            self.visit_arrayrow(node)
        elif node.token.type == Token.Type.Subexpression:
            self.visit_subexpression(node)
        elif node.token.type == Token.Type.Argument:
            self.visit_argument(node)
        elif node.token.type == Token.Type.OperatorPrefix:
            self.visit_operatorprefix(node)
        elif node.token.type == Token.Type.OperatorInfix:
            self.visit_operatorinfix(node)
        elif node.token.type == Token.Type.OperatorPostfix:
            self.visit_operatorpostfix(node)
        else:
            raise ValueError(f"Unexpected token {node.token=}")

    def visit_children(self, node: Node):
        for child in node.children:
            self.visit(child)

    @abstractmethod
    def visit_operand(self, node: Node):
        pass

    @abstractmethod
    def visit_function(self, node: Node):
        pass

    @abstractmethod
    def visit_array(self, node: Node):
        pass

    @abstractmethod
    def visit_arrayrow(self, node: Node):
        pass

    @abstractmethod
    def visit_subexpression(self, node: Node):
        pass

    @abstractmethod
    def visit_operatorprefix(self, node: Node):
        pass

    @abstractmethod
    def visit_operatorinfix(self, node: Node):
        pass

    @abstractmethod
    def visit_operatorpostfix(self, node: Node):
        pass


class NodeToStringVisitor(NodeVisitor):
    """Node visitor that produces a sequence of values for converting an AST back into a string."""

    def __init__(self, prefix: str, separator: str, options: Options):
        self.values = []
        self.prefix = prefix
        self.separator = separator
        self.options = options

    @property
    def value(self):
        if self.values:
            return self.prefix + self.separator + self.separator.join(self.values)
        return ""

    def visit_children(self, node: Node, separator=None):
        for i, child in enumerate(node.children):
            self.visit(child)
            if separator is not None and i+1 < len(node.children):
                self.values.append(separator)

    def visit_operand(self, node: Node):
        self.values.append(node.token.value)

    def visit_function(self, node: Node):
        self.values.append(node.token.value + "(")
        self.visit_children(node, self.options.list_separator)
        self.values.append(")")

    def visit_array(self, node: Node):
        self.values.append(self.options.left_brace)
        self.visit_children(node)
        if self.values[-1] == self.options.row_separator:
            self.values.pop()
        self.values.append(self.options.right_brace)

    def visit_arrayrow(self, node: Node):
        self.visit_children(node, self.options.list_separator)
        self.values.append(self.options.row_separator)

    def visit_subexpression(self, node: Node):
        self.values.append("(")
        self.visit_children(node)
        self.values.append(")")

    def visit_operatorprefix(self, node: Node):
        self.values.append(node.token.value)
        self.visit_children(node)

    def visit_operatorinfix(self, node: Node):
        self.visit_children(node, node.token.value)

    def visit_operatorpostfix(self, node: Node):
        self.visit_children(node)
        self.values.append(node.token.value)


class NodeToTokensVisitor(NodeVisitor):
    """Node visitor that reconstitutes a token stream from an ast"""

    def __init__(self, options: Options):
        self.tokens = []
        self.stack = []
        self.options = options

    def visit(self, node):
        super().visit(node)
        self.maybe_add_list_separator()

    def visit_children(self, node, start=None, end=None):
        self.stack.append(node)
        for child in node.children[start:end]:
            self.visit(child)
        self.stack.pop()

        # If the node's a function remove the last separator token
        if node.token and node.token.type == Token.Type.Function:
            if self.tokens and self.tokens[-1].type == Token.Type.Argument:
                self.tokens.pop()

    def maybe_add_list_separator(self):
        # If we're in a function add a list separator token
        prev_token_type = self.stack[-1].token.type if self.stack and self.stack[-1].token else None
        if prev_token_type == Token.Type.Function:
            self.tokens.append(Token(value=self.options.list_separator,
                                    type=Token.Type.Argument,
                                    sub_type=Token.SubType._None))

    def visit_operand(self, node: Node):
        self.tokens.append(node.token)

    def visit_function(self, node: Node):
        self.tokens.append(Token(value=node.token.value,
                                 type=Token.Type.Function,
                                 sub_type=Token.SubType.Start))

        self.visit_children(node)

        self.tokens.append(Token(value=")",
                                 type=Token.Type.Function,
                                 sub_type=Token.SubType.Stop))

    def visit_array(self, node: Node):
        self.tokens.append(Token(value=self.options.left_brace,
                                 type=Token.Type.Array,
                                 sub_type=Token.SubType.Start))

        self.visit_children(node)

        self.tokens.append(Token(value=self.options.right_brace,
                                 type=Token.Type.Array,
                                 sub_type=Token.SubType.Stop))

    def visit_arrayrow(self, node: Node):
        self.tokens.append(Token(value="",
                                 type=Token.Type.ArrayRow,
                                 sub_type=Token.SubType.Start))

        self.visit_children(node)

        self.tokens.append(Token(value=self.options.row_separator,
                                 type=Token.Type.ArrayRow,
                                 sub_type=Token.SubType.Stop))

    def visit_subexpression(self, node: Node):
        self.tokens.append(Token(value="(",
                                 type=Token.Type.Subexpression,
                                 sub_type=Token.SubType.Start))

        self.visit_children(node)

        self.tokens.append(Token(value=")",
                                 type=Token.Type.Subexpression,
                                 sub_type=Token.SubType.Stop))

    def visit_operatorprefix(self, node: Node):
        self.tokens.append(node.token)
        self.visit_children(node)

    def visit_operatorinfix(self, node: Node):
        self.visit_children(node, end=1)
        self.tokens.append(node.token)
        self.visit_children(node, start=1)

    def visit_operatorpostfix(self, node: Node):
        self.visit_children(node)
        self.tokens.append(node.token)


class ASTBuilder(TokenVisitor):
    """Token vistor that builds the AST from a sequence of tokens."""

    def __init__(self):
        self.__stack = [Node()]

    @property
    def root(self):
        return self.__stack[0]

    @property
    def __prev_token_type(self):
        token = self.__stack[-1].token
        if token:
            return token.type

    def visit_unknown(self, token: Token):
        raise ValueError(f"Unexpected token {token=}")

    def visit_operand(self, token: Token):
        if self.__prev_token_type == Token.Type.OperatorInfix:
            operand = self.__stack.pop()
            operand.children.append(Node(token))
        elif self.__prev_token_type == Token.Type.OperatorPrefix:
            operator = self.__stack.pop()
            operator.children.append(Node(token))
        else:
            self.__stack[-1].children.append(Node(token))

    def visit_function(self, token: Token):
        if token.sub_type == Token.SubType.Start:
            node = Node(token)
            self.__stack[-1].children.append(node)
            self.__stack.append(node)
        elif token.sub_type == Token.SubType.Stop:
            self.__stack.pop()

    def visit_array(self, token: Token):
        if token.sub_type == Token.SubType.Start:
            node = Node(token)
            self.__stack[-1].children.append(node)
            self.__stack.append(node)
        elif token.sub_type == Token.SubType.Stop:
            self.__stack.pop()

    def visit_arrayrow(self, token: Token):
        if token.sub_type == Token.SubType.Start:
            node = Node(token)
            self.__stack[-1].children.append(node)
            self.__stack.append(node)
        elif token.sub_type == Token.SubType.Stop:
            self.__stack.pop()

    def visit_subexpression(self, token: Token):
        if token.sub_type == Token.SubType.Start:
            node = Node(token)
            self.__stack[-1].children.append(node)
            self.__stack.append(node)
        elif token.sub_type == Token.SubType.Stop:
            self.__stack.pop()

    def visit_operatorprefix(self, token: Token):
        operator = Node(token, [])
        self.__stack[-1].children.append(operator)
        self.__stack.append(operator)

    def visit_operatorinfix(self, token: Token):
        lhs = self.__stack[-1].children.pop()
        operator = Node(token, [lhs])
        self.__stack[-1].children.append(operator)
        self.__stack.append(operator)

    def visit_operatorpostfix(self, token: Token):
        lhs = self.__stack[-1].children.pop()
        operator = Node(token, [lhs])
        self.__stack[-1].children.append(operator)

    def visit_argument(self, token: Token):
        # This is the argument separate token, which can be ignored
        pass

    def visit_whitespace(self, token: Token):
        # Ignore whitespace when building the tree
        pass


def build_ast(tokens: list[Token]) -> Node:
    """Build an AST from an Excel formula string."""
    visitor = ASTBuilder()
    for token in tokens:
        visitor.visit(token)
    return visitor.root
