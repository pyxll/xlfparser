from xlfparser import tokenize, Type, SubType, build_ast


def test_tokenize_formula():
    tokens = tokenize("=3*4+5")
    assert len(tokens) == 5
    assert tokens[0].value == "3"
    assert tokens[0].type == Type.Operand
    assert tokens[1].value == "*"
    assert tokens[1].type == Type.OperatorInfix
    assert tokens[2].value == "4"
    assert tokens[2].type == Type.Operand
    assert tokens[3].value == "+"
    assert tokens[3].type == Type.OperatorInfix
    assert tokens[4].value == "5"
    assert tokens[4].type == Type.Operand


def test_tokenize_function():
    tokens = tokenize("=SUM(1, 2)")
    assert len(tokens) == 5
    assert tokens[0].value == "SUM"
    assert tokens[0].type == Type.Function
    assert tokens[0].sub_type == SubType.Start
    assert tokens[1].value == "1"
    assert tokens[1].type == Type.Operand
    assert tokens[2].value == ","
    assert tokens[2].type == Type.Argument
    assert tokens[3].value == "2"
    assert tokens[3].type == Type.Operand
    assert tokens[4].value == ")"
    assert tokens[4].type == Type.Function
    assert tokens[4].sub_type == SubType.Stop


def test_ast_builder_formula():
    ast = build_ast("=3*4+5")
    assert str(ast) == "3 * 4 + 5"


def test_ast_builder_prefix():
    ast = build_ast("=-1")
    assert str(ast) == "- 1"


def test_ast_builder_postfix():
    ast = build_ast("=1%")
    assert str(ast) == "1 %"

def test_ast_builder_function():
    ast = build_ast("=SUM(1, 2)")
    assert str(ast) == "SUM( 1 , 2 )"


def test_ast_builder_subexpression():
    ast = build_ast("=1+(2*-3)")
    assert str(ast) == "1 + ( 2 * - 3 )"


def test_ast_builder_array():
    ast = build_ast("={1,2,3;4,5,6}")
    assert str(ast) == "{ 1 , 2 , 3 ; 4 , 5 , 6 ; }"

def test_ast_builder_complex():
    ast = build_ast("={FUNC(-1, 2*3, 4%, (5 / 6))}")
    assert str(ast) == "{ FUNC( - 1 , 2 * 3 , 4 % , ( 5 / 6 ) ) ; }"
